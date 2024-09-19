#include "MindEyeSoundEnv.h"

MindEyeSoundEnv::MindEyeSoundEnv(
    IXAudio2 *pXAudio2,
    IXAudio2MasteringVoice *pMasterVoice,
    WAVEFORMATEX WFXOut,
    std::function<std::map<DWORD64, MindEyeEmitterInfo>(void)> getEmitterInfo,
    std::function<std::vector<MindEyeListenerInfo>(void)> getListenerInfo)
{
    // 配置主音频
    this->pXAudio2 = pXAudio2;
    this->pMasterVoice = pMasterVoice;

    // 设置输出音频格式
    this->WFXOut = WFXOut;

    // 设置声源音频格式
    this->WFXSrc = WFXOut;
    this->WFXSrc.nChannels = 1;
    this->WFXSrc.nAvgBytesPerSec = this->WFXSrc.nSamplesPerSec * this->WFXSrc.nBlockAlign;

    // 初始化子混音
    DWORD dwOutChannels = WFXOut.nChannels;
    pSubmixVoices.resize(dwOutChannels, 0);
    for (DWORD i = 0; i < dwOutChannels; i++)
    {
        IXAudio2SubmixVoice *pSubmixVoice = pSubmixVoices[i];

        // 创建子混音
        pXAudio2->CreateSubmixVoice(&pSubmixVoice, 1, WFXSrc.nSamplesPerSec, 0, 254);

        // 设置输出对象
        XAUDIO2_SEND_DESCRIPTOR send = {0, pMasterVoice};
        XAUDIO2_VOICE_SENDS sendList = {1, &send};
        pSubmixVoice->SetOutputVoices(&sendList);

        // 设置输出对象
        std::vector<FLOAT> outputMatrix;
        outputMatrix.resize(dwOutChannels, 0);
        outputMatrix[i] = 1.0f;
        pSubmixVoice->SetOutputMatrix(pMasterVoice, 1, dwOutChannels, outputMatrix.data());
    }

    pListeners.resize(dwOutChannels, 0);
    for (auto &pListener : pListeners)
    {
        pListener = new MindEyeListener();
    }

    // 设置函数指针
    this->getEmitterInfo = getEmitterInfo;
    this->getListenerInfo = getListenerInfo;
}

MindEyeSoundEnv::~MindEyeSoundEnv()
{
    for (const auto &emitterEntry : emitterMap)
    {
        delete emitterEntry.second;
    }
    for (const auto &pListener : pListeners)
    {
        delete pListener;
    }
    for (const auto &pSubmixVoice : pSubmixVoices)
    {
        pSubmixVoice->DestroyVoice();
    }
}

HRESULT MindEyeSoundEnv::OnRefresh()
{
    HRESULT hr = S_OK;

    // 获取最新接收器信息
    auto listenerInfos = getListenerInfo();

    // 获取最新发生器信息
    auto emitterInfoMap = getEmitterInfo();

    if (listenerInfos.size() != WFXOut.nChannels)
    {
        return S_FALSE;
    }

    if (emitterInfoMap.size() == 0)
    {
        return S_OK;
    }

    // 更新接收器信息
    for (DWORD i = 0; i < WFXOut.nChannels; i++)
    {
        auto pListener = pListeners[i];
        pListener->spaceInfo = listenerInfos[i].spaceInfo;
    }

    // 音效链
    XAUDIO2_EFFECT_DESCRIPTOR effectDesc = XAUDIO2_EFFECT_DESCRIPTOR();

    // 删除发生器&中间混音
    std::vector<DWORD64> toDeleteEmitterIds;
    for (const auto &emitterEntry : emitterMap)
    {
        auto dwId = emitterEntry.first;
        if (emitterInfoMap.find(dwId) == emitterInfoMap.end())
        {
            toDeleteEmitterIds.push_back(dwId);
        }
    }

    for (const auto &dwId : toDeleteEmitterIds)
    {
        // 删除发生器
        auto pEmitter = emitterMap.at(dwId);
        emitterMap.erase(dwId);
        delete pEmitter;

        // 删除中间混音
        const auto &pMiddleSubmixVoices = pMiddleSubmixVoicesMap.at(dwId);
        for (const auto &pMiddleSubmixVoice : pMiddleSubmixVoices)
        {
            pMiddleSubmixVoice->DestroyVoice();
        }
        pMiddleSubmixVoicesMap.erase(dwId);
    }

    // 新增/更新发生器&中间混音
    for (const auto &emitterInfoEntry : emitterInfoMap)
    {
        auto emitterInfoId = emitterInfoEntry.first;
        auto emitterInfoTmp = emitterInfoEntry.second;
        auto emitterEntry = emitterMap.find(emitterInfoId);
        MindEyeEmitter *pEmitterTmp = NULL;
        // 新增发生器&中间混音
        if (emitterEntry == emitterMap.end())
        {
            // 新增发生器
            pEmitterTmp = new MindEyeEmitter();

            // 设置空间信息
            pEmitterTmp->spaceInfo = emitterInfoTmp.spaceInfo;

            // 设置缓冲区数据
            XAUDIO2_BUFFER buffer = {0};
            buffer.AudioBytes = emitterInfoTmp.bufferSize;
            buffer.pAudioData = emitterInfoTmp.pBufferData;

            // 设置循环播放
            buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

            // 创建发生器声源
            pXAudio2->CreateSourceVoice(&pEmitterTmp->pAudio2SourceVoice, &WFXSrc);

            // 发生器声源设置音频缓冲
            XAUDIO2_BUFFER audioBuffer = XAUDIO2_BUFFER();
            audioBuffer.AudioBytes = emitterInfoTmp.bufferSize;
            audioBuffer.pAudioData = emitterInfoTmp.pBufferData;
            audioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
            pEmitterTmp->pAudio2SourceVoice->SubmitSourceBuffer(&audioBuffer);
            emitterMap.emplace(emitterInfoId, pEmitterTmp);

            // 设置发生器输出对象
            std::vector<XAUDIO2_SEND_DESCRIPTOR> srcSendDescriptors;
            srcSendDescriptors.reserve(WFXOut.nChannels);

            // 新增中间混音
            std::vector<IXAudio2SubmixVoice *> pMiddleSubmixVoices;
            pMiddleSubmixVoices.reserve(WFXOut.nChannels);
            
            pMiddleSubmixVoicesMap.emplace(emitterInfoId, pMiddleSubmixVoices);

            // 设置发生器输出对象
            XAUDIO2_VOICE_SENDS srcSends = {0};
            srcSends.SendCount = WFXOut.nChannels;
            srcSends.pSends = srcSendDescriptors.data();
            pEmitterTmp->pAudio2SourceVoice->SetOutputVoices(&srcSends);

            // 播放音频
            pEmitterTmp->pAudio2SourceVoice->Start();

            // 加入发生器
            emitterMap.emplace(emitterInfoEntry.first, pEmitterTmp);
        }
        // 更新发生器
        else
        {
            pEmitterTmp = (*emitterEntry).second;
        }
    }
    
    return S_OK;
}

DWORD MindEyeSoundEnv::getEnvComplexity()
{
    DWORD dwEmitterNum = (DWORD)emitterMap.size();
    DWORD dwListenerNum = (DWORD)pListeners.size();
    return dwEmitterNum * dwListenerNum;
}
