#pragma once
#include "MindEyeCommon.h"

/**
 * 空间信息.
 */
struct SpaceInfo
{
	// 坐标
	FLOAT PX;
	FLOAT PY;
	FLOAT PZ;
	// 速度
	FLOAT VX;
	FLOAT VY;
	FLOAT VZ;
};

/**
 * 外部传入发生器信息.
 */
struct MindEyeEmitterInfo
{
	/** 发生器空间信息 */
	SpaceInfo spaceInfo;

	/** 发生器缓冲区大小 */
	DWORD bufferSize;

	/** 发生器缓冲区数据 */
	BYTE *pBufferData;
};

/**
 * 内部使用发生器信息.
 */
struct MindEyeEmitter
{
	/** 发生器空间信息 */
	SpaceInfo spaceInfo;

	/** 声源信息 */
	IXAudio2SourceVoice *pAudio2SourceVoice;

	~MindEyeEmitter();
};

/**
 * 外部传入接收器信息.
 */
struct MindEyeListenerInfo
{
	/** 空间信息 */
	SpaceInfo spaceInfo;
};

/**
 * 内部使用接收器信息.
 */
struct MindEyeListener
{
	/** 空间信息 */
	SpaceInfo spaceInfo;
};
