#pragma once
#include "../Common.h"
#include <Windows.h>
#include <mmreg.h>
#include <typeinfo>
#include <vector>

#ifdef _XBOX // Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX // Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

// 查找与读取文件区块
HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD &dwChunkSize, DWORD &dwChunkDataPosition);
HRESULT FindChunk(std::vector<BYTE> &bWaveData, DWORD fourcc, DWORD &dwChunkSize, DWORD &dwChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void *buffer, DWORD buffersize, DWORD bufferoffset);

class MindEyeWave
{
private:
    /** 仅用于类型传参 */
    static constexpr INT8 int8 = 0;
    static constexpr INT16 int16 = 0;
    static constexpr INT32 int32 = 0;
    static constexpr FLOAT float32 = 0;
    static constexpr DOUBLE double64 = 0;

    /**
     * 载入wav文件.
     *
     * \param hFile：载入文件句柄
     * \return S_OK：载入成功
     */
    HRESULT load(HANDLE hFile);

    /**
     * 保存wav文件.
     *
     * \param hFile：保存文件句柄
     * \return S_OK：保存成功
     */
    HRESULT save(HANDLE hFile);

    template <typename CurT, typename TarT>
    BOOL convert(CurT curT, TarT tarT);

public:
    static const DWORD ENCODING_FOEMAT_UNKNOWN = 0x00000000;
    static const DWORD ENCODING_FOEMAT_INT8 = 0x00000001;
    static const DWORD ENCODING_FOEMAT_INT16 = 0x00000002;
    static const DWORD ENCODING_FOEMAT_INT32 = 0x00000003;
    static const DWORD ENCODING_FOEMAT_FLOAT = 0x00000004;
    static const DWORD ENCODING_FOEMAT_DOUBLE = 0x00000005;

    // RIFF区块
    DWORD dwChunkID;
    DWORD dwChunkSize;
    DWORD dwFormat;

    // fmt区块
    DWORD dwFmtChunkID;
    DWORD dwFmtChunkSize;
    WORD wAudioFormat;
    WORD wNumChannels;
    DWORD dwSampleRate;
    DWORD dwByteRate;
    WORD wBlockAlign;
    WORD wBitsPerSample;

    // data区块
    DWORD dwDataChunkID;
    DWORD dwDataChunkSize;
    std::vector<BYTE> bBufferData;

    /**
     * 载入wav文件.
     *
     * \param wFileName：载入文件名
     * \return S_OK：载入成功
     */
    HRESULT load(const wchar_t *wFileName);

    /**
     * 载入wav文件.
     *
     * \param fileName：载入文件名
     * \return S_OK：载入成功
     */
    HRESULT load(const char *fileName);

    /**
     * 保存wav文件.
     *
     * \param wFileName：保存文件名
     * \return S_OK：保存成功
     */
    HRESULT save(const wchar_t *wFileName);

    /**
     * 保存wav文件.
     *
     * \param fileName：保存文件名
     * \return S_OK：保存成功
     */
    HRESULT save(const char *fileName);

    /**
     * 获取wav文件编码格式.
     *
     * \return 成功：UINT8 UINT16 UINT32 FLOAT32 FLOAT64 失败：UNKNOWN
     */
    DWORD getEncodingFormat();

    /**
     * 转换wav文件的格式.
     *
     * \param T 转换目标类型：支持UINT8 UINT16 UINT32 FLOAT32 FLOAT64
     * \return TRUE：转换成功
     */
    template <typename TarT>
    BOOL convert(TarT);

    /**
     * 转换wav文件的格式（根据编码格式ID）.
     *
     * \param dwEncodingFormat：编码格式ID 支持ENCODING_FOEMAT_INT8 ENCODING_FOEMAT_INT16 ENCODING_FOEMAT_INT32 ENCODING_FOEMAT_FLOAT ENCODING_FOEMAT_DOUBLE
     * \return TRUE：转换成功
     */
    BOOL convertById(DWORD dwEncodingFormat);

    /**
     * 获取音频格式.
     *
     * \param WFX：音频格式的引用
     * \return S_OK：获取成功
     */
    HRESULT getWaveFormat(WAVEFORMATEX &WFX);
};

template <typename CurT, typename TarT>
inline BOOL MindEyeWave::convert(CurT, TarT)
{
    // 目标类型与当前类型相同，不需要转换
    if (typeid(CurT) == typeid(TarT))
    {
        return TRUE;
    }

    // 音频格式归一化
    dwChunkSize -= dwDataChunkSize;
    dwByteRate /= sizeof(CurT);
    dwDataChunkSize /= sizeof(CurT);

    // 权重，整形权重为最大值，浮点型权重为1
    UINT64 qwWeightCur = 1;
    UINT64 qwWeightTar = 1;
    if (wAudioFormat == WAVE_FORMAT_PCM)
    {
        qwWeightCur = qwWeightCur << sizeof(CurT);
    }
    if (typeid(TarT) == typeid(INT8) || typeid(TarT) == typeid(INT16) || typeid(TarT) == typeid(INT32))
    {
        wAudioFormat = WAVE_FORMAT_PCM;
        qwWeightTar = qwWeightTar << sizeof(TarT);
    }
    else
    {
        wAudioFormat = WAVE_FORMAT_IEEE_FLOAT;
    }

    // 类型转换
    TarT *tarTBuffer = new TarT[dwDataChunkSize]();
    for (DWORD i = 0; i < dwDataChunkSize; i++)
    {
        CurT curTData = ((CurT *)bBufferData.data())[i];
        tarTBuffer[i] = (TarT)(curTData * 1.0 * qwWeightTar / qwWeightCur);
    }
    dwByteRate *= sizeof(TarT);
    dwDataChunkSize *= sizeof(TarT);
    bBufferData.resize(dwDataChunkSize, 0);
    ::memcpy_s(bBufferData.data(), dwDataChunkSize, tarTBuffer, dwDataChunkSize);
    delete[] tarTBuffer;

    // 音频格式计算
    wBlockAlign = sizeof(TarT) * wNumChannels;
    wBitsPerSample = sizeof(TarT) * 8;
    dwChunkSize += dwDataChunkSize;
    return TRUE;
}

template <typename TarT>
inline BOOL MindEyeWave::convert(TarT tarT)
{
    DWORD dwEncodingFotmat = getEncodingFormat();

    switch (dwEncodingFotmat)
    {
    case ENCODING_FOEMAT_INT8:
        return convert(int8, tarT);
    case ENCODING_FOEMAT_INT16:
        return convert(int16, tarT);
    case ENCODING_FOEMAT_INT32:
        return convert(int32, tarT);
    case ENCODING_FOEMAT_FLOAT:
        return convert(float32, tarT);
    case ENCODING_FOEMAT_DOUBLE:
        return convert(double64, tarT);
    default:
        return FALSE;
    }
}
