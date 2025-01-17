#pragma once
#include "MindEyeCommon.h"

// 默认窗口参数
static const WCHAR DEFAULTWINDOWCLASSNAME[] = L"MindEye";
static const WCHAR DEFAULTWINDOWNAME[] = L"MindEye";

// 声音渲染顺序
static const DWORD PSMAINMIX = 255;

// 默认声音参数
static const DWORD DEFAULTCHANNELS = 2;
static const DWORD DEFAULTSAMPLERATE = 48000;
static const DWORD DEFAULTBLOCKALIGN = 8;
static const DWORD DEFAULTBYTERATE = DEFAULTSAMPLERATE * DEFAULTBLOCKALIGN;

// 自定义消息
static const DWORD MY_PAINT = 0x0800;
static const DWORD MY_REFRESH = 0x0900;
static const DWORD MY_QUIT = 0x0C00;
