#pragma once
#define DEBUG_MODE

#include <Windows.h>
#include <xaudio2.h>
#include <xapobase.h>

#ifdef DEBUG_MODE
#define MYDEBUG(x) x
#else
#define MYDEBUG(x)
#endif

// 全局变量
extern CHAR *logStr;
