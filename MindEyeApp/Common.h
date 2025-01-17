#pragma once
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <functional>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <d3d11.h>

// 通过进程名查找进程ID

DWORD FindPIDByName(LPWSTR wName);

// TODO：查找与读取文件区块

// 远程线程注入

HRESULT RemoteThreadInject32(LPWSTR wProcessName, LPWSTR wDllName);
HRESULT RemoteThreadInject32(DWORD dwPID, LPWSTR wDllName);
HRESULT RemoteThreadInject32(HANDLE hProcess, LPWSTR wDllName);

// 劫持进程注入

HRESULT HijackProcessInject(LPWSTR wProcessName, LPWSTR wDllName);

// 获取进程已加载的模块信息

BOOL GetModuleInfoByName(DWORD dwPID, const WCHAR *wModuleName, MODULEENTRY32 *pMe32);

// 常量
/** 32位LoadLibraryW的地址 */
static const FARPROC LOADLIBRARYW32 = (FARPROC)0x76D298D0;
/** 64位LoadLibraryW的地址 */
static const FARPROC LOADLIBRARYW64 = (FARPROC)0x00007FFAF114EB90;
