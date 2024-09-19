#include "MindEyeFactory.h"

MindEyeFactory::MindEyeFactory(WAVEFORMATEX WFX)
{
	// 设置波形图属性
	this->WFX = WFX;

	// 初始化COM
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// 创建XAudio引擎
	::XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	// 初始化输出声源
	pXAudio2->CreateMasteringVoice(
		&pMasterVoice, WFX.nChannels, WFX.nSamplesPerSec);

	// 设置自动刷新
	OnRefreshParam *pOnRefreshParam = new OnRefreshParam();
	pOnRefreshParam->pMindEyeFactory = this;
	MindEyeTimer::setTimeOut((LPTHREAD_START_ROUTINE)OnRefresh, pOnRefreshParam, 1000);
}

HRESULT MindEyeFactory::addEnv(
	DWORD dwEnvId,
	std::function<std::map<DWORD64, MindEyeEmitterInfo>(void)> getEmitterInfo,
	std::function<std::vector<MindEyeListenerInfo>(void)> getListenerInfo)
{
	// 声音环境已存在则错误返回
	if (MindEyeSoundEnvMap.find(dwEnvId) != MindEyeSoundEnvMap.end())
	{
		return E_ACCESSDENIED;
	}

	// 声音环境不存在则新建并添加声音环境
	MindEyeSoundEnv *pMindEyeSoundEnv =
		new MindEyeSoundEnv(pXAudio2, pMasterVoice, WFX, getEmitterInfo, getListenerInfo);
	MindEyeSoundEnvMap.emplace(dwEnvId, pMindEyeSoundEnv);
	return S_OK;
}

HRESULT MindEyeFactory::removeEnv(DWORD dwEnvId)
{
	auto pMindEyeSoundEnvEntry = MindEyeSoundEnvMap.find(dwEnvId);
	// 声音环境不存在则错误返回
	if (MindEyeSoundEnvMap.find(dwEnvId) == MindEyeSoundEnvMap.end())
	{
		return E_ACCESSDENIED;
	}

	// 声音环境存在则删除并释放内存
	MindEyeSoundEnv *pMindEyeSoundEnv = (*pMindEyeSoundEnvEntry).second;
	MindEyeSoundEnvMap.erase(pMindEyeSoundEnvEntry);
	delete pMindEyeSoundEnv;
	return S_OK;
}

HRESULT WINAPI MindEyeFactory::OnRefresh(OnRefreshParam *pOnRefreshParam)
{
	// 遍历声音环境
	for (const auto &pMindEyeSoundEnvEntry : pOnRefreshParam->pMindEyeFactory->MindEyeSoundEnvMap)
	{
		// 刷新声音环境
		HRESULT hr = pMindEyeSoundEnvEntry.second->OnRefresh();
		if (hr != S_OK)
		{
			return hr;
		}
	}
	return S_OK;
}
