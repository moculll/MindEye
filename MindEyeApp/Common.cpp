#include "Common.h"
#include "PEAnalyze.h"

DWORD findPIDByName(const wchar_t *wProcessName)
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
        if (wcsstr(pe32.szExeFile, wProcessName) != NULL)
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
HRESULT RemoteThreadInject32(const wchar_t *wProcessName, const wchar_t *wDllName)
{
    DWORD dwPID = findPIDByName(wProcessName);
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
HRESULT RemoteThreadInject32(DWORD dwPID, const wchar_t *wDllName)
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
HRESULT RemoteThreadInject32(HANDLE hProcess, const wchar_t *wDllName)
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
HRESULT HijackProcessInject(const wchar_t *wProcessName, const wchar_t *wDllName)
{
    // 返回值
    HRESULT hresult = S_OK;
    BOOL bRet = FALSE;
    // 窗口信息
    STARTUPINFO si = STARTUPINFO();
    // 进程信息
    PROCESS_INFORMATION pi = PROCESS_INFORMATION();

    // 挂起启动进程
    bRet = ::CreateProcessW(
        wProcessName, (LPWSTR)wProcessName, NULL, NULL, TRUE,
        CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    if (!bRet)
    {
        // 启动进程失败返回
        hresult = ::GetLastError();
        return hresult;
    }

    // 远程线程注入
    hresult = RemoteThreadInject32(pi.hProcess, wDllName);
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

BOOL GetModuleInfoByName(DWORD dwPID, const WCHAR *wModuleName, MODULEENTRY32 *pMe32)
{
    // 进程id为NULL代表自身进程
    if (dwPID == NULL)
    {
        dwPID = ::GetCurrentProcessId();
    }

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

VOID ClearConsole()
{
    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = {0, 0};
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwCharsWritten;
    DWORD dwConSize;

    ::GetConsoleScreenBufferInfo(hConsole, &csbi);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    ::FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &dwCharsWritten);
    ::FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &dwCharsWritten);
    ::SetConsoleCursorPosition(hConsole, coordScreen);
    return;
}

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

CHAR Num2SingleChar(DWORD dwNum)
{
    if (dwNum < 10)
    {
        return (CHAR)(dwNum + '0');
    }
    else
    {
        return (CHAR)(dwNum + 'A' - 10);
    }
}

HANDLE MindEyeExceptionHandler::hExceptionHandler = NULL;
BreakPoint *MindEyeExceptionHandler::pLastBreakPoint = NULL;
std::list<BreakPoint> MindEyeExceptionHandler::breakPointList;

LONG NTAPI MindEyeExceptionHandler::ExceptionHandler(_EXCEPTION_POINTERS *ExceptionInfo)
{
    std::list<BreakPoint>::iterator breakPointIter;
    DWORD dwExceptionAdd = (DWORD)ExceptionInfo->ExceptionRecord->ExceptionAddress;
    switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
    {
    // 软件断点
    case EXCEPTION_BREAKPOINT:
    {
        for (breakPointIter = breakPointList.begin(); breakPointIter != breakPointList.end(); breakPointIter++)
        {
            if (breakPointIter->dwType == SOFTBREAK &&
                breakPointIter->dwAddress == dwExceptionAdd)
            {
                // 改回原代码
                DWORD dwProtect;
                ::VirtualProtect((LPVOID)breakPointIter->dwAddress, 1, PAGE_EXECUTE_READWRITE, &dwProtect);
                ::memcpy_s((LPVOID)breakPointIter->dwAddress, 1, &breakPointIter->bCode, 1);
                ::VirtualProtect((LPVOID)breakPointIter->dwAddress, 1, dwProtect, &dwProtect);

                // 执行异常处理
                LONG lRet = breakPointIter->ExceptionHandler(ExceptionInfo);

                // 一次性断点则移除断点
                if (breakPointIter->singleFlg)
                {
                    breakPointList.erase(breakPointIter);
                }
                // 非一次性断点设置单步执行并记录
                else
                {
                    ExceptionInfo->ContextRecord->EFlags |= 0x00000100;
                    pLastBreakPoint = new BreakPoint(*breakPointIter);
                }

                // 回到断点位置执行
                return lRet;
            }
        }
        return EXCEPTION_CONTINUE_SEARCH;
        break;
    }
    // 硬件断点
    case STATUS_SINGLE_STEP:
    {
        // 其他断点引发的单步执行
        if (pLastBreakPoint != NULL)
        {
            DWORD dwProtect;
            BYTE bSoftBreak = '\xCC';
            switch (pLastBreakPoint->dwType)
            {
            case SOFTBREAK:
            {
                // 恢复软件断点
                ::VirtualProtect((LPVOID)pLastBreakPoint->dwAddress, 1, PAGE_EXECUTE_READWRITE, &dwProtect);
                ::memcpy_s((LPVOID)pLastBreakPoint->dwAddress, 1, &bSoftBreak, 1);
                ::VirtualProtect((LPVOID)pLastBreakPoint->dwAddress, 1, dwProtect, &dwProtect);
            }
            }
            delete pLastBreakPoint;
            pLastBreakPoint = NULL;
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        // TODO：硬件断点处理
        return EXCEPTION_CONTINUE_SEARCH;
    }
    // PAGE GUARD
    case EXCEPTION_GUARD_PAGE:
    {
        // TODO：PAGE GUARD处理
        return EXCEPTION_CONTINUE_SEARCH;
    }
    default:
    {
        return EXCEPTION_CONTINUE_SEARCH;
        break;
    }
    }
}

BOOL MindEyeExceptionHandler::AddBreakPoint(DWORD dwBreakPointAdd, PVECTORED_EXCEPTION_HANDLER ExceptionHandler, DWORD dwType, BOOL singleFlg)
{
    BOOL bRet = FALSE;
    std::list<BreakPoint>::iterator breakPointIter;
    BreakPoint breakPoint = BreakPoint(dwType, dwBreakPointAdd, ExceptionHandler, singleFlg);

    // 首次调用注册异常处理函数
    if (hExceptionHandler == NULL)
    {
        hExceptionHandler = ::AddVectoredExceptionHandler(1, MindEyeExceptionHandler::ExceptionHandler);
    }

    // 判断断点是否已存在
    for (breakPointIter = breakPointList.begin(); breakPointIter != breakPointList.end(); breakPointIter++)
    {
        if (breakPointIter->dwAddress == dwBreakPointAdd)
        {
            return FALSE;
        }
    }

    // 判断中断类型
    switch (dwType)
    {
    // 软件断点
    case SOFTBREAK:
    {
        DWORD dwProtect;

        // 插入断点
        ::VirtualProtect((LPVOID)dwBreakPointAdd, 1, PAGE_EXECUTE_READWRITE, &dwProtect);
        breakPoint.bCode = *(BYTE *)dwBreakPointAdd;
        breakPointList.push_back(breakPoint);

        // 添加软件断点
        BYTE bSoftBreak = '\xCC';
        ::memcpy_s((LPVOID)dwBreakPointAdd, 1, &bSoftBreak, 1);
        ::VirtualProtect((LPVOID)dwBreakPointAdd, 1, dwProtect, &dwProtect);
        return TRUE;
    }
    // 硬件断点
    case HARDBREAK:
    {
        // TODO：添加硬件断点
        return FALSE;
    }
    // PAGE GUARD
    case PAGEGUARD:
    {
        // TODO：添加PAGE GUARD
        return FALSE;
    }
    default:
    {
        return FALSE;
    }
    }
}

MindEyeExceptionHandler::MindEyeExceptionHandler()
{
}

MindEyeExceptionHandler::~MindEyeExceptionHandler()
{
}

BreakPoint::BreakPoint()
{
    this->dwType = NULL;
    this->dwAddress = NULL;
    this->ExceptionHandler = NULL;
    this->singleFlg = NULL;
    this->bCode = NULL;
    return;
}

BreakPoint::BreakPoint(DWORD dwBPType, DWORD dwAddress, PVECTORED_EXCEPTION_HANDLER ExceptionHandler, BOOL singleFlg)
{
    this->dwType = dwBPType;
    this->dwAddress = dwAddress;
    this->ExceptionHandler = ExceptionHandler;
    this->singleFlg = singleFlg;
    this->bCode = NULL;
    return;
}
