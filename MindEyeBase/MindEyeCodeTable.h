#pragma once
#include "MindEyeCommon.h"

// 默认窗口参数
static const WCHAR DEFAULTWINDOWCLASSNAME[] = L"MindEye";
static const WCHAR DEFAULTWINDOWNAME[] = L"MindEye";

// 声音渲染顺序
static const DWORD PSMAINMIX = 255;

/** 双声道8位深音频 */
static constexpr WAVEFORMATEX WFX_REF_INT8 = {
    WAVE_FORMAT_PCM,
    2,
    44100,
    44100 * 2 * sizeof(INT8),
    sizeof(INT8),
    sizeof(INT8) * 8,
    sizeof(WAVEFORMATEX)};

/** 双声道16位深音频 */
static constexpr WAVEFORMATEX WFX_REF_INT16 = {
    WAVE_FORMAT_PCM,
    2,
    44100,
    44100 * 2 * sizeof(INT16),
    sizeof(INT16),
    sizeof(INT16) * 8,
    sizeof(WAVEFORMATEX)};

/** 双声道32位深整形音频 */
static constexpr WAVEFORMATEX WFX_REF_INT32 = {
    WAVE_FORMAT_PCM,
    2,
    44100,
    44100 * 2 * sizeof(INT32),
    sizeof(INT32),
    sizeof(INT32) * 8,
    sizeof(WAVEFORMATEX)};

/** 双声道32位深浮点型音频 */
static constexpr WAVEFORMATEX WFX_REF_FLOAT = {
    WAVE_FORMAT_IEEE_FLOAT,
    2,
    44100,
    44100 * 2 * sizeof(FLOAT),
    sizeof(FLOAT),
    sizeof(FLOAT) * 8,
    sizeof(WAVEFORMATEX)};

/** 双声道64位深浮点型音频 */
static constexpr WAVEFORMATEX WFX_REF_DOUBLE = {
    WAVE_FORMAT_IEEE_FLOAT,
    2,
    44100,
    44100 * 2 * sizeof(DOUBLE),
    sizeof(DOUBLE),
    sizeof(DOUBLE) * 8,
    sizeof(WAVEFORMATEX)};
