#include "MindEyeRenderer2D.h"

MindEyeRenderer2D::MindEyeRenderer2D()
{
}

MindEyeRenderer2D::~MindEyeRenderer2D()
{
}

/**
 * 2D音效渲染函数.
 *
 * \param params：MindEyeRenderer2D参数
 * \param pData：音频数据
 * \param cFrames：声音帧数
 * \param cChannels：通道数
 */
void MindEyeRenderer2D::DoProcess(const MindEyeRenderer2DParams &params, FLOAT32 *pData, UINT32 cFrames, UINT32 cChannels)
{
    SpaceInfo spaceInfoEmitter = params.spaceInfoEmitter;
    SpaceInfo spaceInfoListener = params.spaceInfoListener;
    // 时域大小
    DWORD dwSizeTD = cFrames * 2;
    // 频域大小
    DWORD dwSizeFD = dwSizeTD / 2 + 1;
    // 备份上一音频帧的数据
    double* pDataTDBackup = new double[cFrames]{};
    memcpy_s(pDataTDBackup, cFrames * sizeof(*params.pDataTD), params.pDataTD, cFrames * sizeof(*params.pDataTD));
    // 记录当前音频帧的数据
    for (DWORD i = 0; i < cFrames; i++)
    {
        params.pDataTD[i + cFrames] = (double)pData[i];
    }
    // 正向变换
    fftw_execute(*params.pFftPlan);

    // TODO：音频处理

    // 逆向变换
    fftw_execute(*params.pIfftPlan);

    for (DWORD i = 0; i < cFrames; i++)
    {
        pData[i] = (FLOAT32)(params.pDataTD[i + cFrames] / dwSizeTD);
    }
    // 恢复当前帧的数据
    memcpy_s(params.pDataTD, cFrames * sizeof(*params.pDataTD), pDataTDBackup, cFrames * sizeof(*params.pDataTD));
    delete[] pDataTDBackup;
    return;
}
