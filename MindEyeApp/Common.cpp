#include "Common.h"
#include "PEAnalyze.h"

/**
 * 通过进程名查找进程ID.
 *
 * \param wName：进程名（部分包含）
 * \return 非NULL：进程id NULL：匹配失败
 */
DWORD FindPIDByName(LPWSTR wName)
{
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32 = {};
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // 创建进程快照
    hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }
    Process32First(hSnapshot, &pe32);
    do
    {
        // 进程名匹配成功返回进程id
        if (wcsstr(pe32.szExeFile, wName) != NULL)
        {
            ::CloseHandle(hSnapshot);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe32));

    // 进程名匹配失败返回NULL
    ::CloseHandle(hSnapshot);
    return NULL;
}

/**
 * 远程线程注入.
 *
 * \param wProcessName：进程名称（部分）
 * \param wDllName：加载的动态链接库名称
 * \return 正常：S_OK
 */
HRESULT RemoteThreadInject32(LPWSTR wProcessName, LPWSTR wDllName)
{
    DWORD dwPID = FindPIDByName(wProcessName);
    if (dwPID != NULL)
    {
        return RemoteThreadInject32(dwPID, wDllName);
    }
    else
    {
        return E_NOTIMPL;
    }
}

/**
 * 远程线程注入.
 *
 * \param dwPID：进程ID
 * \param wDllName：加载的动态链接库名称
 * \return 正常：S_OK
 */
HRESULT RemoteThreadInject32(DWORD dwPID, LPWSTR wDllName)
{
    MODULEENTRY32 me32 = {0};
    me32.dwSize = sizeof(MODULEENTRY32);
    if (!GetModuleInfoByName(dwPID, L"kernel32.dll", &me32))
    {
        return S_FALSE;
    }
    DWORD dwLoadLibraryWAddress = (DWORD)me32.modBaseAddr;
    PortableExecutable PE = PortableExecutable(me32.szExePath);
    dwLoadLibraryWAddress += PE.GetFunctionAddressByName("LoadLibraryW");

    // 获取进程句柄
    HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);

    // 打开进程失败
    if (hProcess == NULL)
    {
        return S_FALSE;
    }
    // 远程写入动态链接库名称
    LPVOID address = ::VirtualAllocEx(hProcess, NULL, wcslen(wDllName) * sizeof(WCHAR), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (address == NULL)
    {
        return S_FALSE;
    }
    if (!::WriteProcessMemory(hProcess, address, (LPCVOID)wDllName, wcslen(wDllName) * sizeof(WCHAR), NULL))
    {
        return S_FALSE;
    }

    // 创建远程线程
    if (::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)dwLoadLibraryWAddress, address, 0, NULL) == NULL)
    {
        return S_FALSE;
    }
    ::CloseHandle(hProcess);

    return S_OK;
}

/**
 * 远程线程注入.
 *
 * \param hProcess：进程句柄
 * \param wDllName：加载的动态链接库名称
 * \return 正常：S_OK
 */
HRESULT RemoteThreadInject32(HANDLE hProcess, LPWSTR wDllName)
{
    // 远程写入动态链接库名称
    LPVOID address = ::VirtualAllocEx(hProcess, NULL, wcslen(wDllName) * sizeof(WCHAR), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (address == NULL)
    {
        return S_FALSE;
    }
    if (!::WriteProcessMemory(hProcess, address, (LPCVOID)wDllName, wcslen(wDllName) * sizeof(WCHAR), NULL))
    {
        return S_FALSE;
    }

    // 创建远程线程
    if (::CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LOADLIBRARYW32, address, 0, NULL) == NULL)
    {
        return S_FALSE;
    }
    return S_OK;
}

/**
 * 劫持进程注入.
 *
 * \param wProcessName：待启动的进程名
 * \param wDllName：加载的动态链接库名称
 * \return 正常：S_OK
 */
HRESULT HijackProcessInject(LPWSTR wProcessName, LPWSTR wDllName)
{
    // 返回值
    HRESULT hresult = S_OK;
    BOOL bRet = FALSE;
    // 窗口信息
    STARTUPINFO si = STARTUPINFO();
    // 进程信息
    PROCESS_INFORMATION pi = PROCESS_INFORMATION();

    // 挂起启动进程
    bRet = ::CreateProcess(
        wProcessName, wProcessName, NULL, NULL, TRUE,
        CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    if (!bRet)
    {
        // 启动进程失败返回
        hresult = ::GetLastError();
        return hresult;
    }

    // 远程线程注入
    // hresult = RemoteThreadInject32(pi.hProcess, wDllName);
    if (hresult != S_OK)
    {
        // 注入失败，中止进程
        ::TerminateProcess(pi.hProcess, 0);
        return hresult;
    }

    // 恢复进程运行
    hresult = ::ResumeThread(pi.hThread);
    return hresult;
}

/**
 * 获取进程已加载的模块信息.
 *
 * \param dwPID：进程ID
 * \param wModuleName：查找的模块名称（忽略大小写）
 * \param pMe32：查找到的模块信息
 * \return 找到模块返回TRUE 未找到返回FALSE
 */
BOOL GetModuleInfoByName(DWORD dwPID, const WCHAR *wModuleName, MODULEENTRY32 *pMe32)
{
    // 模块名小写缓冲区
    WCHAR wModulePathLower[MAX_PATH] = L"";
    WCHAR wModuleNameLookForLower[MAX_PATH] = L"";
    wcscpy_s(wModuleNameLookForLower, MAX_PATH, wModuleName);
    _wcslwr_s(wModuleNameLookForLower, MAX_PATH);

    // 获取进程快照
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwPID);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    // 遍历句柄
    BOOL bRet = ::Module32First(hSnapshot, pMe32);
    if (bRet)
    {
        do
        {
            // 模块名转小写
            wcscpy_s(wModulePathLower, MAX_PATH, pMe32->szExePath);
            _wcslwr_s(wModulePathLower, MAX_PATH);
            if (wcsstr(wModulePathLower, wModuleNameLookForLower) != NULL)
            {
                // 查找到模块，成功返回
                ::CloseHandle(hSnapshot);
                return TRUE;
            }
        } while (::Module32Next(hSnapshot, pMe32));
    }

    // 未查到到模块，失败返回
    ::CloseHandle(hSnapshot);
    return FALSE;
}
