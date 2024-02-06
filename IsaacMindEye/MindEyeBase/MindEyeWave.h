#pragma once
#include <Windows.h>
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
    /**
     * 载入wav文件.
     *
     * \param hFile：载入文件句柄
     * \return S_OK：载入成功
     */
    HRESULT Load(HANDLE hFile);

    /**
     * 保存wav文件.
     *
     * \param hFile：保存文件句柄
     * \return S_OK：保存成功
     */
    HRESULT Save(HANDLE hFile);

public:
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
    HRESULT Load(const wchar_t *wFileName);

    /**
     * 载入wav文件.
     *
     * \param fileName：载入文件名
     * \return S_OK：载入成功
     */
    HRESULT Load(const char *fileName);

    /**
     * 保存wav文件.
     *
     * \param wFileName：保存文件名
     * \return S_OK：保存成功
     */
    HRESULT Save(const wchar_t *wFileName);

    /**
     * 保存wav文件.
     *
     * \param fileName：保存文件名
     * \return S_OK：保存成功
     */
    HRESULT Save(const char *fileName);

    /**
     * 获取音频格式.
     *
     * \param WFX：音频格式的引用
     * \return S_OK：获取成功
     */
    HRESULT getWaveFormat(WAVEFORMATEX &WFX);
};