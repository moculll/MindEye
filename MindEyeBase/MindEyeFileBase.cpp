#include "MindEyeFileBase.h"

HRESULT MindEyeFileBase::load(const wchar_t *wFileName)
{
    HRESULT hr = S_OK;
    // 打开文件
    HANDLE hFile = ::CreateFileW(
        wFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = ::GetLastError();
        return hr;
    }

    hr = load(hFile);
    ::CloseHandle(hFile);
    return hr;
}

HRESULT MindEyeFileBase::load(const char *fileName)
{
    HRESULT hr = S_OK;
    // 打开文件
    HANDLE hFile = ::CreateFileA(
        fileName, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = ::GetLastError();
        return hr;
    }

    hr = load(hFile);
    ::CloseHandle(hFile);
    return hr;
}

HRESULT MindEyeFileBase::save(const wchar_t *wFileName)
{
    HRESULT hr = S_OK;
    // 打开文件
    HANDLE hFile = ::CreateFileW(
        wFileName, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = ::GetLastError();
        return hr;
    }
    hr = save(hFile);
    ::CloseHandle(hFile);
    return hr;
}

HRESULT MindEyeFileBase::save(const char *fileName)
{
    HRESULT hr = S_OK;
    // 打开文件
    HANDLE hFile = ::CreateFileA(
        fileName, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = ::GetLastError();
        return hr;
    }
    hr = save(hFile);
    ::CloseHandle(hFile);
    return hr;
}
