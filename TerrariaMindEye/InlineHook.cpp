#include "pch.h"
#include "InlineHook.h"

static const char JMP = '\xE9';
static const char NOP = '\x90';

/**
 * Inline Hook.
 *
 * \param fpTargetFunction：被HOOK的函数地址
 * \param fpNewFunction：钩子函数地址
 * \param dwHookBytes：替换的字节数（至少为5）
 * \return 非NULL：HOOK后目标函数地址 NULL：HOOK失败
 */
LPVOID InlineHook::Hook(LPVOID fpTargetFunction, LPVOID fpHookFunction, DWORD dwHookBytes)
{
    // 函数偏移
    int oldFunctionOffset, newFunctionOffset;
    // 页保护属性
    DWORD oldProtect;

    // 少于5字节，HOOK失败
    if (dwHookBytes < 5)
    {
        return NULL;
    }

    // 保存被HOOK的函数
    LPVOID fpOriginalFunction = ::VirtualAlloc(0, dwHookBytes + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    ::memcpy(fpOriginalFunction, fpTargetFunction, dwHookBytes);
    ::memcpy((LPVOID)((DWORD)fpOriginalFunction + dwHookBytes), &JMP, 1);
    oldFunctionOffset = (int)fpTargetFunction - (int)fpOriginalFunction - 5;
    ::memcpy((LPVOID)((DWORD)fpOriginalFunction + dwHookBytes + 1), &oldFunctionOffset, 4);

    // 改变页保护属性
    VirtualProtect(fpTargetFunction, dwHookBytes, PAGE_EXECUTE_READWRITE, &oldProtect);
    // 跳转至新函数
    ::memcpy(fpTargetFunction, &JMP, 1);
    newFunctionOffset = (int)fpHookFunction - (int)fpTargetFunction - 5;
    ::memcpy((LPVOID)((DWORD)fpTargetFunction + 1), &newFunctionOffset, 4);
    // 补足NOP
    for (DWORD i = 5; i < dwHookBytes; i++)
    {
        ::memcpy((LPVOID)((DWORD)fpTargetFunction + i), &NOP, 1);
    }
    // 恢复页保护属性
    VirtualProtect(fpTargetFunction, dwHookBytes, oldProtect, &oldProtect);

    return fpOriginalFunction;
}
