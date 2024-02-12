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
		MindEyeWaveManager* pMindEyeWaveManager;
		DWORD dwId;
	};
	static HRESULT WINAPI load(LoadParams* pLoadParams);
	char* jsonDir;

	CRITICAL_SECTION cs;
	std::map<DWORD, std::string> WaveMap;
	std::map<DWORD, HANDLE> LoadingMap;
	std::map<DWORD, MindEyeWave> LoadedMap;

public:
	HRESULT init(const char* configPath);
	HRESULT get(DWORD dwId, MindEyeWave** ppWaveFile);
	void destroy();
};
