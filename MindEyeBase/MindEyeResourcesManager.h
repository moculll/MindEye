#pragma once
#include "MindEyeCommon.h"
#include <filesystem>
#include <map>
#include <string>
#include <json.h>

#include "MindEyeWave.h"

class IMindEyeResourcesManager
{
public:
	/**
	 * 初始化资源管理器.
	 *
	 * \param configPath：配置文件的路径
	 * \return S_OK：初始化成功
	 */
	virtual HRESULT init(const char *configPath) = 0;

	/**
	 * 手动设置资源文件根目录.
	 *
	 * \param rootDir：资源文件根目录
	 * \return TRUE：设置成功 FALSE：未初始化
	 */
	virtual BOOL setRootDir(const char *rootDir) = 0;

	/**
	 * 获取资源（异步）.
	 *
	 * \param dwId：资源ID
	 * \param ppResourcesFile：（输出）资源指针
	 * \return S_OK：获取资源成功 ERROR_IO_PENDING：加载中 S_FALSE：未初始化 E_ACCESSDENIED：资源不存在
	 */
	virtual HRESULT get(DWORD dwId, VOID **ppResourcesFile) = 0;

	/**
	 * 销毁资源管理器.
	 *
	 */
	virtual void destroy() = 0;
};

class MindEyeWaveManager
	: public IMindEyeResourcesManager
{
private:
	struct LoadParams
	{
		MindEyeWaveManager *pMindEyeWaveManager;
		DWORD dwId;
	};
	static HRESULT WINAPI load(LoadParams *pLoadParams);

	/** 关键段（用于线程同步） */
	CRITICAL_SECTION cs;

	/** 音频文件根目录 */
	std::string rootDir;

	/** 音频文件编码格式 */
	DWORD dwEncodingFormat;

	/** ID对应音频路径表 */
	std::map<DWORD, std::string> WaveMap;

	/** 加载中的音频列表 */
	std::map<DWORD, HANDLE> LoadingMap;

	/** 加载完的音频列表 */
	std::map<DWORD, MindEyeWave> LoadedMap;

public:
	/**
	 * 初始化资源管理器.
	 *
	 * \param configPath：配置文件的路径
	 * \return S_OK：初始化成功
	 */
	HRESULT init(const char *configPath) override;

	/**
	 * 手动设置音频文件根目录.
	 *
	 * \param rootDir：资源文件根目录
	 * \return TRUE：设置成功 FALSE：未初始化
	 */
	BOOL setRootDir(const char *rootDir) override;

	/**
	 * 手动设置音频编码格式（加载的音频格式会被强制转换成目标格式）.
	 *
	 * \param dwEncodingFormat：编码格式ID 支持ENCODING_FOEMAT_INT8 ENCODING_FOEMAT_INT16 ENCODING_FOEMAT_INT32 ENCODING_FOEMAT_FLOAT ENCODING_FOEMAT_DOUBLE
	 * \return TRUE：设置成功 FALSE：未初始化
	 */
	BOOL setEncodingFormat(DWORD dwEncodingFormat);

	/**
	 * 获取资源（异步）.
	 *
	 * \param dwId：资源ID
	 * \param ppResourcesFile：（输出）音频资源指针
	 * \return S_OK：获取资源成功 ERROR_IO_PENDING：加载中 S_FALSE：未初始化 E_ACCESSDENIED：资源不存在
	 */
	HRESULT get(DWORD dwId, VOID **ppResourcesFile) override;

	/**
	 * 销毁资源管理器.
	 *
	 */
	void destroy() override;
};
