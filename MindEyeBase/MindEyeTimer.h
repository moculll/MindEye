#pragma once
#include "MindEyeCommon.h"

class MindEyeTimer
{
private:
	struct ThreadProcParam
	{
		LPTHREAD_START_ROUTINE lpStartAddress;
		LPVOID lpParameter;
		DWORD dwInterval;
		DWORD dwCycles;
	};
	/**
	 * 定时任务入口地址.
	 * 
	 * \param pThreadProcParam：定时任务参数
	 * \return 
	 */
	static DWORD WINAPI ThreadProc(ThreadProcParam *pThreadProcParam);

	struct TimerProcParam
	{
		HANDLE hEvent;
		DWORD dwInterval;
	};
	/**
	 * 定时器入口地址.
	 * 
	 * \param pTimerProcParam：定时器参数
	 * \return 
	 */
	static DWORD WINAPI TimerProc(TimerProcParam *pTimerProcParam);

public:
	/**
	 * 每隔一定时间执行一次函数.
	 *
	 * \param lpStartAddress：函数入口
	 * \param lpParameter：函数参数
	 * \param dwInterval：执行间隔（毫秒）
	 * \param dwCycles：循环执行次数，INFINITE 无限循环
	 * \return S_OK：线程创建成功
	 */
	static HRESULT setTimeOut(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwInterval, DWORD dwCycles = INFINITE);
};
