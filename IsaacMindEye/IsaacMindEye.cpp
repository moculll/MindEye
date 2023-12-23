#include "pch.h"
#include "Common.h"
#include "NewFunctions.h"
#include "InlineHook.h"

// 日志初始化
extern std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt("basic_logger", "logs/isaac_mindeye_log.txt");

//
DWORD dwNvoglvBaseAdd = NULL;
DWORD dwNvOglObj = NULL;

// HOOK
VOID MindEyeMain();
BOOL Hook();
BOOL Unhook();
DWORD GetNvOglObj();
LONG NTAPI EHGetNvOglObj(_EXCEPTION_POINTERS *ExceptionInfo);
BOOL CALLBACK FindWindowByNameProc(HWND hwnd, LPARAM lParam);

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MindEyeMain, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

VOID MindEyeMain()
{
    BOOL bRet = TRUE;
    HMODULE hModSelf = NULL;
    ::GetModuleHandleEx(NULL, L"IsaacMindEye.dll", &hModSelf);

    // 日志初始化
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::err);
    spdlog::flush_every(std::chrono::seconds(5));

    // HOOK
    if (!Hook())
    {
        // 卸载DLL
        ::MessageBox(0, L"HOOK失败", L"ERROR", 0);
        ::FreeLibraryAndExitThread(hModSelf, 0);
        return;
    }
    GetNvOglObj();

    return;
}

LPVOID lpSwapBuffers = NULL;
LPVOID lpGlDrawElements = NULL;
LPVOID lpGlBindTexture = NULL;

BOOL Hook()
{
    // 加载DLL
    HMODULE hModGdi = ::LoadLibraryA("gdi32full.dll");
    HMODULE hModOpengl = ::LoadLibraryA("opengl32.dll");

    // HOOK SwapBuffers函数
    lpSwapBuffers = (LPVOID)::GetProcAddress(hModGdi, "SwapBuffers");
    if (lpSwapBuffers == NULL)
    {
        // 卸载DLL并删除opengl环境
        ::FreeLibrary(hModGdi);
        ::FreeLibrary(hModOpengl);
        return FALSE;
    }
    NewFunctions::fpSwapBuffers = (NewFunctions::SWAPBUFFERS)InlineHook::Hook(lpSwapBuffers, NewFunctions::SwapBuffers, 7);
    if (NewFunctions::fpSwapBuffers == NULL)
    {
        // 卸载DLL并删除opengl环境
        ::FreeLibrary(hModGdi);
        ::FreeLibrary(hModOpengl);
        return FALSE;
    }

    // HOOK glDrawElements函数
    lpGlDrawElements = (LPVOID)::GetProcAddress(hModOpengl, "glDrawElements");
    if (lpGlDrawElements == NULL)
    {
        // 卸载DLL并删除opengl环境
        ::FreeLibrary(hModGdi);
        ::FreeLibrary(hModOpengl);
        return FALSE;
    }
    NewFunctions::fpGlDrawElements = (NewFunctions::GLDRAWELEMENTS)InlineHook::Hook(lpGlDrawElements, NewFunctions::glDrawElements, 5);
    if (NewFunctions::fpGlDrawElements == NULL)
    {
        // 卸载DLL并删除opengl环境
        ::FreeLibrary(hModGdi);
        ::FreeLibrary(hModOpengl);
        return FALSE;
    }

    // HOOK glBindTexture函数
    lpGlBindTexture = (LPVOID)::GetProcAddress(hModOpengl, "glBindTexture");
    if (lpGlBindTexture == NULL)
    {
        // 卸载DLL并删除opengl环境
        ::FreeLibrary(hModGdi);
        ::FreeLibrary(hModOpengl);
        return FALSE;
    }
    NewFunctions::fpGlBindTexture = (NewFunctions::GLBINDTEXTURE)InlineHook::Hook(lpGlBindTexture, NewFunctions::glBindTexture, 5);
    if (NewFunctions::fpGlBindTexture == NULL)
    {
        // 卸载DLL并删除opengl环境
        ::FreeLibrary(hModGdi);
        ::FreeLibrary(hModOpengl);
        return FALSE;
    }

    // 卸载DLL并删除opengl环境
    ::FreeLibrary(hModGdi);
    ::FreeLibrary(hModOpengl);
    return TRUE;
}

BOOL Unhook()
{
    // 解除HOOK SwapBuffers函数
    if (!InlineHook::Unhook(lpSwapBuffers, NewFunctions::fpSwapBuffers, 7))
    {
        return FALSE;
    }
    NewFunctions::fpSwapBuffers = NULL;

    // 解除HOOK glDrawElements函数
    if (!InlineHook::Unhook(lpGlDrawElements, NewFunctions::fpGlDrawElements, 5))
    {
        return FALSE;
    }
    NewFunctions::fpGlDrawElements = NULL;

    // 解除HOOK glBindTexture函数
    if (!InlineHook::Unhook(lpGlBindTexture, NewFunctions::fpGlBindTexture, 5))
    {
        return FALSE;
    }
    NewFunctions::fpGlBindTexture = NULL;

    return TRUE;
}

DWORD GetNvOglObj()
{
    // 获取nvoglv32.dll基址
    MODULEENTRY32 me32 = {};
    me32.dwSize = sizeof(MODULEENTRY32);
    DWORD dwPID = ::GetCurrentProcessId();
    GetModuleInfoByName(dwPID, L"nvoglv32.dll", &me32);
    dwNvoglvBaseAdd = (DWORD)me32.modBaseAddr;
    logger->info("nvoglvBaseAdd: {:#8X}", dwNvoglvBaseAdd);

    HANDLE hEvent = ::CreateEvent(NULL, TRUE, FALSE, L"GetNvOglObj");

    // 添加断点
    MindEyeExceptionHandler::AddBreakPoint(dwNvoglvBaseAdd + 0x0019C000, EHGetNvOglObj, MindEyeExceptionHandler::SOFTBREAK, TRUE);

    // 等待获取NvOgl对象
    ::WaitForSingleObject(hEvent, INFINITE);
    ::CloseHandle(hEvent);

    return dwNvOglObj;
}

LONG NTAPI EHGetNvOglObj(_EXCEPTION_POINTERS *ExceptionInfo)
{
    __asm
    {
        PUSH ESI;
        MOV ESI, FS: [0x00000BF0];
        MOV dwNvOglObj, ESI;
        POP ESI;
    }
    logger->info("nvglObj: {:#8X}", dwNvOglObj);

    // 获取NvOgl对象事件同步
    HANDLE hEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, L"GetNvOglObj");
    ::SetEvent(hEvent);
    ::CloseHandle(hEvent);

    return EXCEPTION_CONTINUE_EXECUTION;
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
        windowMatchStruct->successFlg = TRUE;
        return FALSE;
    }
    return TRUE;
}
