#pragma once
#include "MindEyeCommon.h"
#include <map>
#include <string>
#include <json.h>

#include "MindEyeWave.h"

class MindEyeWaveManager
{
private:
	struct LoadParams
	{
		MindEyeWaveManager *pMindEyeWaveManager;
		DWORD dwId;
	};
	static HRESULT WINAPI Load(LoadParams *pLoadParams);

	CRITICAL_SECTION cs;
	std::map<DWORD, std::string> WaveMap;
	std::map<DWORD, HANDLE> LoadingMap;
	std::map<DWORD, MindEyeWave> LoadedMap;

public:
	HRESULT Init(const char *configPath);
	HRESULT Get(DWORD dwId, MindEyeWave **ppWaveFile);
	void Destroy();
};
