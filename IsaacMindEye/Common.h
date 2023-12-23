#pragma once
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <set>
#include <map>
#include <list>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <opengl/glew.h>

// 通过进程名查找进程ID

DWORD FindPIDByName(LPWSTR wName);

// 获取进程已加载的模块信息

BOOL GetModuleInfoByName(DWORD dwPID, const WCHAR *wModuleName, MODULEENTRY32 *pMe32);

struct WindowMatchStruct
{
    const WCHAR *wWindowName = NULL;
    HWND hWnd = NULL;
    BOOL successFlg = FALSE;
};

struct BreakPoint
{
    DWORD dwType;                                 // 断点类型
    DWORD dwAddress;                              // 断点地址
    PVECTORED_EXCEPTION_HANDLER ExceptionHandler; // 异常处理函数
    BOOL singleFlg;                               // 一次性标志
    BYTE bCode;                                   // 断点原代码

    BreakPoint();
    BreakPoint(DWORD dwBPType, DWORD dwAddress, PVECTORED_EXCEPTION_HANDLER ExceptionHandler, BOOL singleFlg);
};

class MindEyeExceptionHandler
{
private:
    static HANDLE hExceptionHandler;
    static BreakPoint *pLastBreakPoint;
    static std::list<BreakPoint> breakPointList;

    static LONG NTAPI ExceptionHandler(_EXCEPTION_POINTERS *ExceptionInfo);

public:
    static const DWORD SOFTBREAK = 0x00000001;
    static const DWORD HARDBREAK = 0x00000002;
    static const DWORD PAGEGUARD = 0x00000003;

    static BOOL AddBreakPoint(DWORD dwBreakPointAdd, PVECTORED_EXCEPTION_HANDLER ExceptionHandler, DWORD dwType, BOOL singleFlg);

private:
    MindEyeExceptionHandler();
    ~MindEyeExceptionHandler();
};

// 全局变量
extern std::shared_ptr<spdlog::logger> logger;
extern DWORD g_dwRenderThreadId; // 渲染线程id
extern HANDLE g_hRenderThread;   // 渲染线程句柄
