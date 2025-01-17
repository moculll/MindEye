#include "pch.h"
#include "Common.h"
#include "NewFunctions.h"
#include "InlineHook.h"

// 日志初始化
extern std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt("basic_logger", "logs/terraria_mindeye_log.txt");

// HOOK
BOOL Hook();
BOOL CALLBACK FindWindowByNameProc(HWND hwnd, LPARAM lParam);

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Hook, NULL, 0, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

BOOL Hook()
{
    // 日志初始化
    spdlog::flush_on(spdlog::level::err);

    // 查找窗口
    WindowMatchStruct windowMatchStruct = {};
    windowMatchStruct.wWindowName = L"泰拉瑞亚";
    windowMatchStruct.hWnd = NULL;
    ::EnumWindows(FindWindowByNameProc, (LPARAM)&windowMatchStruct);

    // D3D对象
    IDirect3D9 *lpD3d = Direct3DCreate9(D3D_SDK_VERSION);
    // D3D设备
    IDirect3DDevice9 *lpD3dDevice = NULL;
    // 用于初始化D3D设备
    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.hDeviceWindow = windowMatchStruct.hWnd;
    // 初始化D3D设备
    lpD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpD3dDevice);
    // D3D虚表
    LPVOID *D3DVirtualTable = (LPVOID *)*(DWORD *)lpD3dDevice;
    // HOOK EndScene函数 下标42
    NewFunctions::fpD3dEndScene = (NewFunctions::D3DENDSCENE)InlineHook::Hook(D3DVirtualTable[42], &NewFunctions::D3dEndScene, 7);

    // HOOK DrawIndexedPrimitive函数 下标82
    NewFunctions::fpD3dDrawIndexedPrimitive = (NewFunctions::D3DDRAWINDEXEDPRIMITIVE)InlineHook::Hook(D3DVirtualTable[82], &NewFunctions::D3dDrawIndexedPrimitive, 7);

    return TRUE;
}

/**
 * 查找窗口回调.
 *
 * \param hwnd：当前遍历的窗口句柄
 * \param lParam：使用WindowMatchStruct结构体
 * \return FALSE时停止
 */
BOOL CALLBACK FindWindowByNameProc(HWND hwnd, LPARAM lParam)
{
    WindowMatchStruct *windowMatchStruct = (WindowMatchStruct *)lParam;

    WCHAR wWindowName[MAX_PATH] = L"";
    ::GetWindowText(hwnd, wWindowName, MAX_PATH);
    if (wcsstr(wWindowName, windowMatchStruct->wWindowName))
    {
        windowMatchStruct->hWnd = hwnd;
        return FALSE;
    }
    return TRUE;
}
