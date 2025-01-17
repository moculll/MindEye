#include "Common.h"

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
 * 获取进程已加载的模块信息.
 *
 * \param dwPID：进程ID，NULL代表自身进程
 * \param wModuleName：查找的模块名称（忽略大小写）
 * \param pMe32：查找到的模块信息
 * \return 找到模块返回TRUE 未找到返回FALSE
 */
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

/**
 * 控制台清屏.
 *
 * \return 无
 */
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

/**
 * 数字转为单字符显示.
 *
 * \param dwNum：数字
 * \return：单字符 数字0-9转为字符0-9 数字10-35转为字符A-Z
 */
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
