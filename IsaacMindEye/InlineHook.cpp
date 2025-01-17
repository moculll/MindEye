#include "InlineHook.h"

static const char JMP = '\xE9';
static const char NOP = '\x90';

/**
 * Inline Hook.
 *
 * \param fpTargetFunction：被HOOK的函数地址
 * \param fpNewFunction：钩子函数地址
 * \param dwBytes：替换的字节数（至少为5）
 * \return 非NULL：HOOK后目标函数地址 NULL：HOOK失败
 */
LPVOID InlineHook::Hook(LPVOID fpTargetFunction, LPVOID fpHookFunction, DWORD dwBytes)
{
    // 函数偏移
    int oldFunctionOffset, newFunctionOffset;
    // 页保护属性
    DWORD oldProtect;

    // 少于5字节，HOOK失败
    if (dwBytes < 5)
    {
        return NULL;
    }

    // 保存被HOOK的函数
    LPVOID fpOriginalFunction = ::VirtualAlloc(0, dwBytes + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    ::memcpy(fpOriginalFunction, fpTargetFunction, dwBytes);
    ::memcpy((LPVOID)((DWORD)fpOriginalFunction + dwBytes), &JMP, 1);
    oldFunctionOffset = (int)fpTargetFunction - (int)fpOriginalFunction - 5;
    ::memcpy((LPVOID)((DWORD)fpOriginalFunction + dwBytes + 1), &oldFunctionOffset, 4);

    // 改变页保护属性
    ::VirtualProtect(fpTargetFunction, dwBytes, PAGE_EXECUTE_READWRITE, &oldProtect);
    // 跳转至新函数
    ::memcpy(fpTargetFunction, &JMP, 1);
    newFunctionOffset = (int)fpHookFunction - (int)fpTargetFunction - 5;
    ::memcpy((LPVOID)((DWORD)fpTargetFunction + 1), &newFunctionOffset, 4);
    // 补足NOP
    for (DWORD i = 5; i < dwBytes; i++)
    {
        ::memcpy((LPVOID)((DWORD)fpTargetFunction + i), &NOP, 1);
    }
    // 恢复页保护属性
    ::VirtualProtect(fpTargetFunction, dwBytes, oldProtect, &oldProtect);

    return fpOriginalFunction;
}

/**
 * 解除Inline hook.
 *
 * \param fpTargetFunction：待恢复的函数地址
 * \param fpOriginalFunction：用于恢复的函数地址
 * \param dwBytes：替换的字节数
 * \return TRUE：成功解除HOOK
 */
BOOL InlineHook::Unhook(LPVOID fpTargetFunction, LPVOID fpOriginalFunction, DWORD dwBytes)
{
    // 页保护属性
    DWORD oldProtect;

    // 改变页保护属性
    if (!::VirtualProtect(fpTargetFunction, dwBytes, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        return FALSE;
    }
    // 恢复函数
    ::memcpy(fpTargetFunction, fpOriginalFunction, dwBytes);
    // 恢复页保护属性
    if (!::VirtualProtect(fpTargetFunction, dwBytes, oldProtect, &oldProtect))
    {
        return FALSE;
    }
    // 释放内存
    if (!::VirtualFree(fpOriginalFunction, 0, MEM_RELEASE))
    {
        return FALSE;
    }
    return TRUE;
}
