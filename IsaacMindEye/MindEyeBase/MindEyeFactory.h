#pragma once
#include "MindEyeCommon.h"
#include <map>
#include <functional>

#include "MindEyeDto.h"
#include "MindEyeCodeTable.h"
#include "MindEyeSoundEnv.h"

struct CreateWindowInDto
{
	HWND hWnd;
	HANDLE hEvent;
};

class MindEyeFactory
{
private:
	std::map<DWORD, MindEyeSoundEnv *> MindEyeSoundEnvMap;
	HRESULT OnRefresh();

	static int APIENTRY createMsgWindow(CreateWindowInDto *pCreateWindowInDto);
	static int APIENTRY createMsgSender(MindEyeFactory mindEye);
	static ATOM registAudioPlayerClass(HINSTANCE hInstance);
	static LRESULT CALLBACK AudioPlayerWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

protected:
	HANDLE hPlayer;
	HANDLE hMsgSender;
	HWND hWndPlayer;

	// 输出音频格式
	WAVEFORMATEX WFX;
	// 主引擎
	IXAudio2 *pXAudio2;
	// 主音频
	IXAudio2MasteringVoice *pMasterVoice;

public:
	MindEyeFactory(WAVEFORMATEX WFX);
	~MindEyeFactory();

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
		std::function<std::map<DWORD64, MindEyeListenerInfo>(void)> getListenerInfo);

	/**
	 * 删除声音环境.
	 *
	 * \param dwEnvId：待删除环境ID
	 * \return：成功删除返回S_OK
	 */
	HRESULT removeEnv(DWORD dwEnvId);

	HWND getHWndPlayer();
};