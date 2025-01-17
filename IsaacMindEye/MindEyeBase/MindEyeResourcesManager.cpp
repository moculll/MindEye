#include "MindEyeResourcesManager.h"

HRESULT MindEyeWaveManager::Load(LoadParams *pLoadParams)
{
    MindEyeWave mindEyeWave = {0};
    CHAR *fileName = (CHAR *)pLoadParams->pMindEyeWaveManager->WaveMap[pLoadParams->dwId].data();
    HRESULT hr = mindEyeWave.Load(fileName);
    ::EnterCriticalSection(&pLoadParams->pMindEyeWaveManager->cs);
    pLoadParams->pMindEyeWaveManager->LoadingMap.erase(pLoadParams->dwId);
    if (hr == S_OK)
    {
        pLoadParams->pMindEyeWaveManager->LoadedMap.emplace(pLoadParams->dwId, mindEyeWave);
    }
    ::LeaveCriticalSection(&pLoadParams->pMindEyeWaveManager->cs);
    delete pLoadParams;
    return hr;
}

HRESULT MindEyeWaveManager::Init(WCHAR *wConfigPath)
{
    ::InitializeCriticalSection(&cs);

    HANDLE hFile = ::CreateFileW(wConfigPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return ::GetLastError();
    }

    std::vector<CHAR> jsonStr;
    DWORD dwFileSize = ::GetFileSize(hFile, NULL);
    jsonStr.resize(dwFileSize + 1, 0);
    DWORD dwReadedBytes = 0;
    BOOL bRet = ::ReadFile(hFile, jsonStr.data(), dwFileSize, &dwReadedBytes, NULL);
    ::CloseHandle(hFile);
    if (!bRet)
    {
        return ::GetLastError();
    }
    Json::Value jsonData;
    Json::Reader jsonReader;
    bRet = jsonReader.parse(jsonStr.data(), jsonData);
    if (!bRet)
    {
        return S_FALSE;
    }
    for (Json::String key : jsonData.getMemberNames())
    {
        DWORD dwResourceId = std::stoul(key.c_str());
        std::string resourcePath = jsonData[key].asString().c_str();
        WaveMap.emplace(dwResourceId, resourcePath);
    }
    return S_OK;
}

HRESULT MindEyeWaveManager::Get(DWORD dwId, MindEyeWave **ppWaveFile)
{
    ::EnterCriticalSection(&cs);
    if (WaveMap.find(dwId) == WaveMap.end())
    {
        ::LeaveCriticalSection(&cs);
        return S_FALSE;
    }
    if (LoadedMap.find(dwId) != LoadedMap.end())
    {
        *ppWaveFile = &LoadedMap[dwId];
        ::LeaveCriticalSection(&cs);
        return S_OK;
    }
    if (LoadingMap.find(dwId) == LoadingMap.end())
    {
        LoadParams *pLoadParams = new LoadParams();
        pLoadParams->pMindEyeWaveManager = this;
        pLoadParams->dwId = dwId;
        HANDLE hThread = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Load, (LPVOID)pLoadParams, NULL, NULL);
        if (hThread == NULL)
        {
            delete pLoadParams;
            ::LeaveCriticalSection(&cs);
            return ::GetLastError();
        }
        LoadingMap.emplace(dwId, hThread);
    }
    ::LeaveCriticalSection(&cs);
    return ERROR_IO_PENDING;
}

void MindEyeWaveManager::Destroy()
{
    WaveMap.clear();
    LoadingMap.clear();
    LoadedMap.clear();
    ::DeleteCriticalSection(&cs);
    return;
}
