#pragma once
#include <stdio.h>
#include <Windows.h>

namespace InlineHook
{
	LPVOID Hook(LPVOID fpTargetFunction, LPVOID fpNewFunction, DWORD dwBytes);

	BOOL Unhook(LPVOID fpTargetFunction, LPVOID fpOriginalFunction, DWORD dwBytes);
}
