#include "MindEyeSoundEnv.h"

/**
 * 构造声音环境.
 *
 * \param pXAudio2：XAudio引擎
 * \param pMasterVoice：主音频
 * \param WFXOut：主音频格式
 * \param getEmitterInfo：获取音频发生器信息函数
 * \param getListenerInfo：获取音频接收器信息函数
 */
MindEyeSoundEnv::MindEyeSoundEnv(
    IXAudio2 *pXAudio2,
    IXAudio2MasteringVoice *pMasterVoice,
    WAVEFORMATEX WFXOut,
    std::function<std::map<DWORD64, MindEyeEmitterInfo>(void)> getEmitterInfo,
    std::function<std::map<DWORD64, MindEyeListenerInfo>(void)> getListenerInfo)
{
    HRESULT hr = S_OK;

    // 配置主音频
    MindEyeSoundEnv::pXAudio2 = pXAudio2;
    MindEyeSoundEnv::pMasterVoice = pMasterVoice;

    // 设置声源音频格式
    MindEyeSoundEnv::WFXOut = WFXOut;

    // 设置输出音频格式
    MindEyeSoundEnv::WFXSrc = MindEyeSoundEnv::WFXOut;
    MindEyeSoundEnv::WFXSrc.nChannels = 1;
    MindEyeSoundEnv::WFXSrc.nAvgBytesPerSec =
        MindEyeSoundEnv::WFXSrc.nSamplesPerSec * MindEyeSoundEnv::WFXSrc.nBlockAlign;

    // 创建左右声道音频
    pSubmixVoiceLeft = NULL;
    hr = pXAudio2->CreateSubmixVoice(
        &pSubmixVoiceLeft, 1, WFXSrc.nSamplesPerSec,
        0, 254);

    pSubmixVoiceRight = NULL;
    hr = pXAudio2->CreateSubmixVoice(
        &pSubmixVoiceRight, 1, WFXSrc.nSamplesPerSec,
        0, 254);

    // 设置左右声道音频与主音频映射
    XAUDIO2_SEND_DESCRIPTOR send = {0, pMasterVoice};
    XAUDIO2_VOICE_SENDS sendList = {1, &send};
    hr = pSubmixVoiceLeft->SetOutputVoices(&sendList);
    hr = pSubmixVoiceRight->SetOutputVoices(&sendList);
    FLOAT outputMatrix[2] = {};
    outputMatrix[0] = 1.0f;
    outputMatrix[1] = 0.0f;
    pSubmixVoiceLeft->SetOutputMatrix(pMasterVoice, 1, 2, outputMatrix);
    outputMatrix[0] = 0.0f;
    outputMatrix[1] = 1.0f;
    pSubmixVoiceRight->SetOutputMatrix(pMasterVoice, 1, 2, outputMatrix);

    // 设置函数指针
    MindEyeSoundEnv::getEmitterInfo = getEmitterInfo;
    MindEyeSoundEnv::getListenerInfo = getListenerInfo;

    // 时域大小
    DWORD dwSizeTD = WFXOut.nSamplesPerSec / 100 * 2;
    MindEyeSoundEnv::pDataTD = new double[dwSizeTD]{};
    // 频域大小
    DWORD dwSizeFD = dwSizeTD / 2 + 1;
    MindEyeSoundEnv::pDataFD = new fftw_complex[dwSizeFD]{};
    // 创建FFT计划
    MindEyeSoundEnv::fftPlan = fftw_plan_dft_r2c_1d(dwSizeTD, pDataTD, pDataFD, FFTW_ESTIMATE);
    MindEyeSoundEnv::ifftPlan = fftw_plan_dft_c2r_1d(dwSizeTD, pDataFD, pDataTD, FFTW_ESTIMATE);
}

MindEyeSoundEnv::~MindEyeSoundEnv()
{
    auto mindEyeEmitterEntry = emitterMap.begin();
    while (mindEyeEmitterEntry != emitterMap.end())
    {
        MindEyeEmitter *pMindEyeEmitter = (*mindEyeEmitterEntry).second;
        delete pMindEyeEmitter;
        mindEyeEmitterEntry++;
    }
    emitterMap.clear();
    auto mindEyeListenerEntry = listenerMap.begin();
    while (mindEyeListenerEntry != listenerMap.end())
    {
        MindEyeListener *pMindEyeListener = (*mindEyeListenerEntry).second;
        delete pMindEyeListener;
        mindEyeListenerEntry++;
    }
    listenerMap.clear();
    if (pSubmixVoiceLeft != NULL)
    {
        pSubmixVoiceLeft->DestroyVoice();
    }
    if (pSubmixVoiceRight != NULL)
    {
        pSubmixVoiceRight->DestroyVoice();
    }
    delete[] pDataTD, pDataFD;
    fftw_destroy_plan(fftPlan);
    fftw_destroy_plan(ifftPlan);
}

/**
 * 刷新声音环境.
 *
 * \return
 */
HRESULT MindEyeSoundEnv::OnRefresh()
{
    HRESULT hr = S_OK;

    // 获取最新发生器信息
    std::map<DWORD64, MindEyeEmitterInfo> emitterInfoMap = getEmitterInfo();

    // 获取最新接收器信息
    std::map<DWORD64, MindEyeListenerInfo> listenerInfoMap = getListenerInfo();

    if (emitterInfoMap.size() == 0 || listenerInfoMap.size() == 0)
    {
        return hr;
    }
    // 音效链
    XAUDIO2_EFFECT_DESCRIPTOR effectDesc = XAUDIO2_EFFECT_DESCRIPTOR();

    // 音效参数
    MindEyeRenderer2DParams rendererParams = MindEyeRenderer2DParams();
    rendererParams.pFftPlan = &fftPlan;
    rendererParams.pIfftPlan = &ifftPlan;
    rendererParams.pDataTD = pDataTD;
    rendererParams.pDataFD = pDataFD;

    // 发生器信息
    auto emitterInfoEntry = emitterInfoMap.begin();
    // 发生器
    auto emitterEntry = emitterMap.begin();
    // 接收器信息
    auto listenerInfoEntry = listenerInfoMap.begin();
    // 接收器
    auto listenerEntry = listenerMap.begin();

    // 删除多余的发生器
    while (emitterEntry != emitterMap.end())
    {
        DWORD64 dwId = (*emitterEntry).first;
        MindEyeEmitter *pEmitter = (*emitterEntry).second;
        emitterEntry++;
        if (emitterInfoMap.find(dwId) == emitterInfoMap.end())
        {
            emitterMap.erase(dwId);
            delete pEmitter;
        }
    }

    // 新增、更新发生器
    while (emitterInfoEntry != emitterInfoMap.end())
    {
        MindEyeEmitter *pEmitterTmp;
        MindEyeEmitterInfo emitterInfoTmp = (*emitterInfoEntry).second;
        emitterEntry = emitterMap.find((*emitterInfoEntry).first);
        if (emitterEntry == emitterMap.end())
        {
            // 音频数据缓冲区
            XAUDIO2_BUFFER audioBuffer = XAUDIO2_BUFFER();
            audioBuffer.AudioBytes = emitterInfoTmp.bufferSize;
            audioBuffer.pAudioData = emitterInfoTmp.pBufferData;
            audioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
            // 新增发生器
            pEmitterTmp = new MindEyeEmitter();
            // 设置空间信息
            pEmitterTmp->spaceInfo = emitterInfoTmp.spaceInfo;
            // 设置缓冲区数据
            pEmitterTmp->buffer.AudioBytes = emitterInfoTmp.bufferSize;
            pEmitterTmp->buffer.pAudioData = emitterInfoTmp.pBufferData;
            // 设置循环播放
            pEmitterTmp->buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

            // 创建左声道声源
            hr = pXAudio2->CreateSourceVoice(
                &pEmitterTmp->pAudio2SourceVoiceL, &WFXSrc);
            // 左声道声源设置音频缓冲
            pEmitterTmp->pAudio2SourceVoiceL->SubmitSourceBuffer(&audioBuffer);
            // 左声道声源->左声道混音
            pEmitterTmp->sendL.pOutputVoice = pSubmixVoiceLeft;
            pEmitterTmp->sendlistL.SendCount = 1;
            pEmitterTmp->sendlistL.pSends = &pEmitterTmp->sendL;
            hr = pEmitterTmp->pAudio2SourceVoiceL->SetOutputVoices(&pEmitterTmp->sendlistL);
            // 创建音效
            MindEyeRenderer2D *pRendererL = NULL;
            MindEyeRenderer2D::CreateInstance(NULL, 0, &pRendererL);
            effectDesc.InitialState = true;
            effectDesc.OutputChannels = 1;
            effectDesc.pEffect = static_cast<IXAPO *>(pRendererL);
            // 设置音效
            pEmitterTmp->effectChainL.EffectCount = 1;
            pEmitterTmp->effectChainL.pEffectDescriptors = &effectDesc;
            hr = pEmitterTmp->pAudio2SourceVoiceL->SetEffectChain(&pEmitterTmp->effectChainL);
            pRendererL->Release();
            // 设置音效参数
            rendererParams.spaceInfoEmitter = pEmitterTmp->spaceInfo;
            rendererParams.spaceInfoEmitter.PX = 1;
            listenerInfoEntry = listenerInfoMap.begin();
            rendererParams.spaceInfoListener = (*listenerInfoEntry).second.spaceInfo;
            listenerInfoEntry++;
            hr = pEmitterTmp->pAudio2SourceVoiceL->SetEffectParameters(0, &rendererParams, sizeof MindEyeRenderer2DParams);

            // 创建右声道声源
            hr = pXAudio2->CreateSourceVoice(
                &pEmitterTmp->pAudio2SourceVoiceR, &WFXSrc);
            // 右声道声源设置音频缓冲
            pEmitterTmp->pAudio2SourceVoiceR->SubmitSourceBuffer(&audioBuffer);
            // 右声道声源->右声道混音
            pEmitterTmp->sendR.pOutputVoice = pSubmixVoiceLeft;
            pEmitterTmp->sendlistR.SendCount = 1;
            pEmitterTmp->sendlistR.pSends = &pEmitterTmp->sendR;
            hr = pEmitterTmp->pAudio2SourceVoiceR->SetOutputVoices(&pEmitterTmp->sendlistR);
            // 创建音效
            MindEyeRenderer2D *pRendererR = NULL;
            MindEyeRenderer2D::CreateInstance(NULL, 0, &pRendererR);
            effectDesc.InitialState = true;
            effectDesc.OutputChannels = 1;
            effectDesc.pEffect = static_cast<IXAPO *>(pRendererR);
            // 设置音效
            pEmitterTmp->effectChainR.EffectCount = 1;
            pEmitterTmp->effectChainR.pEffectDescriptors = &effectDesc;
            hr = pEmitterTmp->pAudio2SourceVoiceR->SetEffectChain(&pEmitterTmp->effectChainR);
            pRendererR->Release();
            // 设置音效参数
            rendererParams.spaceInfoEmitter = pEmitterTmp->spaceInfo;
            rendererParams.spaceInfoEmitter.PX = 2;
            rendererParams.spaceInfoListener = (*listenerInfoEntry).second.spaceInfo;
            listenerInfoEntry++;
            hr = pEmitterTmp->pAudio2SourceVoiceR->SetEffectParameters(0, &rendererParams, sizeof MindEyeRenderer2DParams);
            // 播放音频
            pEmitterTmp->pAudio2SourceVoiceL->Start();
            pEmitterTmp->pAudio2SourceVoiceR->Start();
            // 加入发生器
            emitterMap[(*emitterInfoEntry).first] = pEmitterTmp;
        }
        else
        {
            pEmitterTmp = (*emitterEntry).second;
            // 更新发生器音效参数
            rendererParams.spaceInfoEmitter = emitterInfoTmp.spaceInfo;
            listenerInfoEntry = listenerInfoMap.begin();
            // 左声道
            rendererParams.spaceInfoListener = (*listenerInfoEntry).second.spaceInfo;
            listenerInfoEntry++;
            pEmitterTmp->pAudio2SourceVoiceL->SetEffectParameters(0, &rendererParams, sizeof MindEyeRenderer2DParams);

            // 右声道
            rendererParams.spaceInfoListener = (*listenerInfoEntry).second.spaceInfo;
            listenerInfoEntry++;
            pEmitterTmp->pAudio2SourceVoiceR->SetEffectParameters(0, &rendererParams, sizeof MindEyeRenderer2DParams);
        }
        emitterInfoEntry++;
    }
    return S_OK;
}

/**
 * 获取环境复杂度，用于多线程优化.
 *
 * \return：声音环境复杂度
 */
DWORD MindEyeSoundEnv::getEnvComplexity()
{
    DWORD dwEmitterNum = (DWORD)emitterMap.size();
    DWORD dwListenerNum = (DWORD)listenerMap.size();
    return dwEmitterNum * dwListenerNum;
}
