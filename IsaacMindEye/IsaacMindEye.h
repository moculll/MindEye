#pragma once
#include "Common.h"
#include "NewFunctions.h"
#include "InlineHook.h"
#include "Hook.h"
#include "Timer.h"
#include "IsaacGame/IsaacGame.h"
#include "MindEyeBase/MindEyeFactory.h"
#include "MindEyeBase/MindEyeWave.h"

WNDPROC oldConsoleProc = NULL;
std::set<WPARAM> pressedKeySet;
IsaacGame *pIsaacGame;
BOOL g_resourcesReadyFlg;

VOID MindEyeMain();
VOID LoadResources();
BOOL Hook();
BOOL Unhook();
LRESULT ChangeShowModeProc(HWND, UINT, WPARAM, LPARAM);

std::map<DWORD64, MindEyeEmitterInfo> getEmitterInfo();
std::map<DWORD64, MindEyeListenerInfo> getListenerInfo();