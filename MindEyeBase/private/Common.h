#pragma once
#define DEBUG_MODE
#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <opengl/glew.h>

#ifdef DEBUG_MODE
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif // DEBUG_MODE

/**
 * 通过进程名查找进程ID.
 *
 * \param wName：进程名（部分包含）
 * \return 非NULL：进程id NULL：匹配失败
 */
DWORD findPIDByName(LPWSTR wName);

/**
 * 获取进程已加载的模块信息.
 *
 * \param dwPID：进程ID，NULL代表自身进程
 * \param wModuleName：查找的模块名称（忽略大小写）
 * \param pMe32：查找到的模块信息
 * \return 找到模块返回TRUE 未找到返回FALSE
 */
BOOL GetModuleInfoByName(DWORD dwPID, const WCHAR *wModuleName, MODULEENTRY32 *pMe32);

/**
 * 控制台清屏.
 *
 * \return 无
 */
void ClearConsole();

/**
 * 查找窗口回调.
 *
 * \param hwnd：当前遍历的窗口句柄
 * \param lParam：使用WindowMatchStruct结构体
 * \return FALSE时停止
 */
BOOL CALLBACK FindWindowByNameProc(HWND hwnd, LPARAM lParam);

/**
 * 数字转为单字符显示.
 *
 * \param dwNum：数字
 * \return：单字符 数字0-9转为字符0-9 数字10-35转为字符A-Z
 */
CHAR Num2SingleChar(DWORD dwNum);

/**
 * 大小端转换.
 *
 * \param pT：待转换的数组
 * \param dwBufferSize：数组长度（字节单位）
 * \return TRUE：转换成功
 */
template <typename T>
void endianConvert(T *pT, DWORD dwLength)
{
    DWORD dwSize = sizeof(T);
    T *tmpList = new T[dwLength]();
    for (DWORD i = 0; i < dwLength; i++)
    {
        for (DWORD j = 0; j < dwSize; j++)
        {
            ((BYTE *)tmpList)[i * dwSize + j] = ((BYTE *)pT)[i * dwSize + dwSize - 1 - j];
        }
    }
    ::memcpy_s(pT, dwLength * sizeof(T), tmpList, dwLength * sizeof(T));
    delete[] tmpList;
}

/**
 * 字符串分割.
 *
 * \param strIn：待分割字符串
 * \param seprator：分隔符
 * \param strsOut：分割结果
 */
void splitStr(std::string &strIn, const char seprator, std::vector<std::string> &strsOut);

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
