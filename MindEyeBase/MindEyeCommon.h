#pragma once
#define DEBUG_MODE

#include <stdio.h>
#include <Windows.h>
#include <xaudio2.h>
#include <mmreg.h>
#include <xapobase.h>

#ifdef DEBUG_MODE
#define MYDEBUG(x) x
#else
#define MYDEBUG(x)
#endif
