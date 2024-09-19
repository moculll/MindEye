#pragma once
#include "MindEyeCommon.h"
#include <map>
#include <functional>

#include "MindEyeDto.h"


class MindEyeSoundEnv
{
private:
	/** 声源音频格式 */
	WAVEFORMATEX WFXSrc;
	/** 输出音频格式 */
	WAVEFORMATEX WFXOut;
	/** XAudio引擎 */
	IXAudio2 *pXAudio2;
	/** 主音频 */
	IXAudio2MasteringVoice *pMasterVoice;
	/** 输出混音 */
	std::vector<IXAudio2SubmixVoice *> pSubmixVoices;
	/** 音频发生器 */
	std::map<DWORD64, MindEyeEmitter *> emitterMap;
	/** 音频接收器 */
	std::vector<MindEyeListener *> pListeners;
	/** 中间混音 */
	std::map<DWORD64, std::vector<IXAudio2SubmixVoice *>> pMiddleSubmixVoicesMap;

	/** 函数指针：获取音频发生器信息 */
	std::function<std::map<DWORD64, MindEyeEmitterInfo>(void)> getEmitterInfo;
	/** 函数指针：获取音频接收器信息 */
	std::function<std::vector<MindEyeListenerInfo>(void)> getListenerInfo;

protected:
public:
	/**
	 * 构造声音环境.
	 *
	 * \param pXAudio2：XAudio引擎
	 * \param pMasterVoice：主音频
	 * \param WFXOut：主音频格式
	 * \param getEmitterInfo：获取音频发生器信息函数
	 * \param getListenerInfo：获取音频接收器信息函数
	 */
	MindEyeSoundEnv(
		IXAudio2 *pXAudio2, IXAudio2MasteringVoice *pMasterVoice, WAVEFORMATEX WFXOut,
		std::function<std::map<DWORD64, MindEyeEmitterInfo>(void)> getEmitterInfo,
		std::function<std::vector<MindEyeListenerInfo>(void)> getListenerInfo);
	~MindEyeSoundEnv();

	/**
	 * 刷新声音环境.
	 *
	 * \return
	 */
	HRESULT OnRefresh();

	/**
	 * 获取环境复杂度，用于多线程优化.
	 *
	 * \return：声音环境复杂度
	 */
	DWORD getEnvComplexity();
};