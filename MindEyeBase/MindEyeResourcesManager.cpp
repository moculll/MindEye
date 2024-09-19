#include "MindEyeResourcesManager.h"

HRESULT MindEyeWaveManager::load(LoadParams *pLoadParams)
{
    MindEyeWave mindEyeWave = MindEyeWave();
    std::string fileName = pLoadParams->pMindEyeWaveManager->WaveMap[pLoadParams->dwId];

    // 绝对路径读取资源
    HRESULT hr = mindEyeWave.load(fileName.data());
    if (hr == ERROR_PATH_NOT_FOUND)
    {
        // 相对路径读取资源
        fileName = pLoadParams->pMindEyeWaveManager->rootDir + "/" + fileName;
        hr = mindEyeWave.load(fileName.data());
    }

    // 编码格式强制转换
    if (hr == S_OK)
    {
        switch (pLoadParams->pMindEyeWaveManager->dwEncodingFormat)
        {
        case MindEyeWave::ENCODING_FOEMAT_INT8:
        case MindEyeWave::ENCODING_FOEMAT_INT16:
        case MindEyeWave::ENCODING_FOEMAT_INT32:
        case MindEyeWave::ENCODING_FOEMAT_FLOAT:
        case MindEyeWave::ENCODING_FOEMAT_DOUBLE:
            mindEyeWave.convertById(pLoadParams->pMindEyeWaveManager->dwEncodingFormat);
            break;
        default:
            break;
        }
    }

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

HRESULT MindEyeWaveManager::init(const char *configPath)
{
    // 初始化关键段
    ::InitializeCriticalSection(&cs);

    // 默认不打开音频编码格式强制转换
    dwEncodingFormat = NULL;

    HANDLE hFile = ::CreateFileA(configPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
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

    // 保存配置文件所在文件夹为根目录
    std::filesystem::path path = configPath;
    rootDir = path.parent_path().string();
    return S_OK;
}

BOOL MindEyeWaveManager::setRootDir(const char *rootDir)
{
    // 未初始化
    if (WaveMap.size() == 0)
    {
        return FALSE;
    }

    // 设置音频文件根目录
    this->rootDir = rootDir;
    return TRUE;
}

BOOL MindEyeWaveManager::setEncodingFormat(DWORD dwEncodingFormat)
{
    // 未初始化
    if (WaveMap.size() == 0)
    {
        return FALSE;
    }

    this->dwEncodingFormat = dwEncodingFormat;
    return TRUE;
}

HRESULT MindEyeWaveManager::get(DWORD dwId, VOID **ppResourcesFile)
{
    // 未初始化
    if (WaveMap.size() == 0)
    {
        return S_FALSE;
    }

    MindEyeWave **ppWaveFile = (MindEyeWave **)ppResourcesFile;
    ::EnterCriticalSection(&cs);
    if (WaveMap.find(dwId) == WaveMap.end())
    {
        ::LeaveCriticalSection(&cs);
        return E_ACCESSDENIED;
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
        HANDLE hThread = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)load, (LPVOID)pLoadParams, NULL, NULL);
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

void MindEyeWaveManager::destroy()
{
    WaveMap.clear();
    LoadingMap.clear();
    LoadedMap.clear();
    ::DeleteCriticalSection(&cs);
    return;
}
