#include "MindEyeTimer.h"

DWORD WINAPI MindEyeTimer::ThreadProc(ThreadProcParam* pThreadProcParam)
{
	// 事件名称 MindEyeTimer进程id_线程id
	::EnterCriticalSection(&pThreadProcParam->cs);
	CHAR eventName[MAX_PATH] = "";
	DWORD dwProcessId = ::GetCurrentProcessId();
	DWORD dwThreadId = ::GetCurrentThreadId();
	sprintf_s(eventName, MAX_PATH, "MindEyeTimer%8X_%8X", dwProcessId, dwThreadId);
	HANDLE hEvent = ::CreateEventA(NULL, FALSE, FALSE, eventName);
	::LeaveCriticalSection(&pThreadProcParam->cs);
	LPTHREAD_START_ROUTINE lpStartAddress = pThreadProcParam->lpStartAddress;
	LPVOID lpParameter = pThreadProcParam->lpParameter;
	DWORD dwCycles = pThreadProcParam->dwCycles;
	while (dwCycles > 0)
	{
		::WaitForSingleObject(hEvent, INFINITE);
		lpStartAddress(lpParameter);
		if (dwCycles != INFINITE)
		{
			dwCycles--;
		}
	}
	::CloseHandle(hEvent);
	delete pThreadProcParam;
	return 0;
}

DWORD WINAPI MindEyeTimer::TimerProc(TimerProcParam* pTimerProcParam)
{
	CHAR eventName[MAX_PATH] = "";
	DWORD dwProcessId = ::GetCurrentProcessId();
	sprintf_s(eventName, MAX_PATH, "MindEyeTimer%8X_%8X", dwProcessId, pTimerProcParam->dwThreadId);
	::EnterCriticalSection(&pTimerProcParam->cs);
	HANDLE hEvent = ::OpenEventA(EVENT_ALL_ACCESS, TRUE, eventName);
	::LeaveCriticalSection(&pTimerProcParam->cs);
	::DeleteCriticalSection(&pTimerProcParam->cs);
	while (GetHandleInformation(hEvent, NULL))
	{
		Sleep(pTimerProcParam->dwInterval);
		::SetEvent(hEvent);
	}
	delete pTimerProcParam;
	return 0;
}

HRESULT MindEyeTimer::setTimeOut(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwInterval, DWORD dwCycles)
{
	DWORD dwThreadId = 0;
	CRITICAL_SECTION cs;
	::InitializeCriticalSection(&cs);

	ThreadProcParam* pThreadProcParam = new ThreadProcParam();
	pThreadProcParam->cs = cs;
	pThreadProcParam->lpStartAddress = lpStartAddress;
	pThreadProcParam->lpParameter = lpParameter;
	pThreadProcParam->dwCycles = dwCycles;
	HANDLE handle = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadProc, pThreadProcParam, NULL, &dwThreadId);
	if (handle == NULL)
	{
		return ::GetLastError();
	}

	TimerProcParam* pTimerProcParam = new TimerProcParam();
	pTimerProcParam->cs = cs;
	pTimerProcParam->dwInterval = dwInterval;
	pTimerProcParam->dwThreadId = dwThreadId;
	handle = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)TimerProc, pTimerProcParam, NULL, NULL);
	if (handle == NULL)
	{
		return ::GetLastError();
	}
	return S_OK;
}
