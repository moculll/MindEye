#include "MindEyeTimer.h"

DWORD WINAPI MindEyeTimer::ThreadProc(ThreadProcParam* pThreadProcParam)
{
	// 事件名称 MindEyeTimer进程id_线程id
	CHAR eventName[MAX_PATH] = "";
	DWORD dwProcessId = ::GetCurrentProcessId();
	DWORD dwThreadId = ::GetCurrentThreadId();
	sprintf_s(eventName, MAX_PATH, "MindEyeTimer%08X_%08X", dwProcessId, dwThreadId);

	// 创建事件
	HANDLE hEvent = ::CreateEventA(NULL, FALSE, FALSE, eventName);
	if (hEvent == NULL)
	{
		return -1;
	}
	LPTHREAD_START_ROUTINE lpStartAddress = pThreadProcParam->lpStartAddress;
	LPVOID lpParameter = pThreadProcParam->lpParameter;
	DWORD dwCycles = pThreadProcParam->dwCycles;

	// 设置定时器参数
	TimerProcParam* pTimerProcParam = new TimerProcParam();
	pTimerProcParam->hEvent = hEvent;
	pTimerProcParam->dwInterval = pThreadProcParam->dwInterval;

	// 启动定时器
	HANDLE handle = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)TimerProc, pTimerProcParam, NULL, NULL);
	if (handle == NULL)
	{
		return -1;
	}
	// 等待定时器激活事件
	while (dwCycles > 0)
	{
		::WaitForSingleObject(hEvent, INFINITE);
		lpStartAddress(lpParameter);

		// 任务的剩余执行次数
		if (dwCycles != INFINITE)
		{
			dwCycles--;
		}
	}

	// 执行完成关闭事件句柄
	::CloseHandle(hEvent);
	delete pThreadProcParam;
	return 0;
}

DWORD WINAPI MindEyeTimer::TimerProc(TimerProcParam* pTimerProcParam)
{
	HANDLE hEvent = pTimerProcParam->hEvent;
	DWORD dwFlags = 0;
	
	// 句柄有效期间，每隔dwInterval毫秒激活一次事件
	while (GetHandleInformation(hEvent, &dwFlags))
	{
		Sleep(pTimerProcParam->dwInterval);
		::SetEvent(hEvent);
	}
	delete pTimerProcParam;
	return 0;
}

HRESULT MindEyeTimer::setTimeOut(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwInterval, DWORD dwCycles)
{
	ThreadProcParam* pThreadProcParam = new ThreadProcParam();
	pThreadProcParam->lpStartAddress = lpStartAddress;
	pThreadProcParam->lpParameter = lpParameter;
	pThreadProcParam->dwInterval = dwInterval;
	pThreadProcParam->dwCycles = dwCycles;
	HANDLE handle = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadProc, pThreadProcParam, NULL, NULL);
	if (handle == NULL)
	{
		return ::GetLastError();
	}
	return S_OK;
}
