#pragma once
#include "MindEyeCommon.h"
#include <map>
#include <functional>

#include "MindEyeDto.h"
#include "MindEyeCodeTable.h"
#include "MindEyeSoundEnv.h"
#include "MindEyeTimer.h"

struct CreateWindowInDto
{
	HWND hWnd;
	HANDLE hEvent;
};

class MindEyeFactory
{
private:
	struct OnRefreshParam
	{
		MindEyeFactory *pMindEyeFactory;
	};
	static HRESULT WINAPI OnRefresh(OnRefreshParam *pOnRefreshParam);

	std::map<DWORD, MindEyeSoundEnv *> MindEyeSoundEnvMap;

protected:
	/** 输出音频格式 */
	WAVEFORMATEX WFX;

	/** 主引擎 */
	IXAudio2 *pXAudio2;

	/** 主音频 */
	IXAudio2MasteringVoice *pMasterVoice;

public:
	/**
	 * 构造函数.
	 *
	 * \param WFX：输出音频格式，默认为双声道32位深浮点型音频
	 */
	MindEyeFactory(WAVEFORMATEX WFX = WFX_REF_FLOAT);

	/**
	 * 添加声音环境.
	 *
	 * \param dwEnvId：待添加环境ID
	 * \param getEmitterInfo：获取音频发生器信息函数
	 * \param getListenerInfo：获取音频接收器信息函数
	 * \return：成功添加返回S_OK
	 */
	HRESULT addEnv(
		DWORD dwEnvId,
		std::function<std::map<DWORD64, MindEyeEmitterInfo>(void)> getEmitterInfo,
		std::function<std::vector<MindEyeListenerInfo>(void)> getListenerInfo);

	/**
	 * 删除声音环境.
	 *
	 * \param dwEnvId：待删除环境ID
	 * \return：成功删除返回S_OK
	 */
	HRESULT removeEnv(DWORD dwEnvId);
};