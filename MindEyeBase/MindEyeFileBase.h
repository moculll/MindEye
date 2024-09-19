#pragma once
#include "Common.h"

class MindEyeFileBase
{
protected:
	/**
	 * 载入wav文件.
	 *
	 * \param hFile：载入文件句柄
	 * \return S_OK：载入成功
	 */
	virtual HRESULT load(HANDLE hFile) = 0;

	/**
	 * 保存wav文件.
	 *
	 * \param hFile：保存文件句柄
	 * \return S_OK：保存成功
	 */
	virtual HRESULT save(HANDLE hFile) = 0;

public:
	/**
	 * 载入wav文件.
	 *
	 * \param wFileName：载入文件名
	 * \return S_OK：载入成功
	 */
	HRESULT load(const wchar_t *wFileName = NULL);

	/**
	 * 载入wav文件.
	 *
	 * \param fileName：载入文件名
	 * \return S_OK：载入成功
	 */
	HRESULT load(const char *fileName = NULL);

	/**
	 * 保存wav文件.
	 *
	 * \param wFileName：保存文件名
	 * \return S_OK：保存成功
	 */
	HRESULT save(const wchar_t *wFileName = NULL);

	/**
	 * 保存wav文件.
	 *
	 * \param fileName：保存文件名
	 * \return S_OK：保存成功
	 */
	HRESULT save(const char *fileName = NULL);
};
