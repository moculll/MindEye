#pragma once
#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
#include <d3d9.h>

struct WindowMatchStruct
{
    const WCHAR* wWindowName;
    HWND hWnd;
};
