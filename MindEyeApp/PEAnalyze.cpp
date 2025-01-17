#include "PEAnalyze.h"

/**
 * .
 *
 * \param dwRVA
 * \param sectionHeaderList
 * \param dwSectionCount
 * \return
 */
DWORD RVA2FOA(DWORD dwRVA, IMAGE_SECTION_HEADER *sectionHeaderList, DWORD dwSectionCount)
{
    for (DWORD i = 0; i < dwSectionCount; i++)
    {
        if (dwRVA >= sectionHeaderList[i].VirtualAddress &&
            dwRVA < sectionHeaderList[i].VirtualAddress + sectionHeaderList[i].SizeOfRawData)
        {
            return dwRVA - sectionHeaderList[i].VirtualAddress + sectionHeaderList[i].PointerToRawData;
        }
    }
    return 0;
}

/**
 * 载入导出表.
 *
 * \param dwBaseAddress：PE文件的当前内存基址
 * \return 无返回值
 */
VOID PortableExecutable::LoadExportFunctions(DWORD64 dwBaseAddress)
{
    // 载入导出表索引
    exportDirectory = *(IMAGE_EXPORT_DIRECTORY *)(dwBaseAddress + RVA2FOA(ntHeader32.OptionalHeader.DataDirectory[0].VirtualAddress));

    // 载入导出表
    if (exportDirectory.NumberOfFunctions != 0)
    {
        exportFunctionList = new ExportFunction[exportDirectory.NumberOfFunctions]();
        // 载入导出函数地址
        for (DWORD i = 0; i < exportDirectory.NumberOfFunctions; i++)
        {
            exportFunctionList[i].dwFunctionAddress = *(DWORD *)(dwBaseAddress + RVA2FOA(exportDirectory.AddressOfFunctions) + 4 * i);
        }
        // 载入导出函数名
        for (DWORD i = 0; i < exportDirectory.NumberOfNames; i++)
        {
            WORD wFunctionIndex = *(WORD *)(dwBaseAddress + RVA2FOA(exportDirectory.AddressOfNameOrdinals) + 2 * i);
            CHAR *pFunctionName = (CHAR *)(dwBaseAddress + RVA2FOA(*(DWORD *)(dwBaseAddress + RVA2FOA(exportDirectory.AddressOfNames) + 4 * i)));
            strcpy_s(exportFunctionList[wFunctionIndex].functionName, MAX_PATH, pFunctionName);
        }
    }
}

/**
 * RVA转FOA.
 *
 * \param dwRVA：RVA值
 * \return FOA值
 */
DWORD PortableExecutable::RVA2FOA(DWORD dwRVA)
{
    for (DWORD i = 0; i < ntHeader32.FileHeader.NumberOfSections; i++)
    {
        if (dwRVA >= sectionHeaderList[i].VirtualAddress &&
            dwRVA < sectionHeaderList[i].VirtualAddress + sectionHeaderList[i].SizeOfRawData)
        {
            return dwRVA - sectionHeaderList[i].VirtualAddress + sectionHeaderList[i].PointerToRawData;
        }
    }
    return 0;
}

/**
 * 根据PE文件名载入.
 *
 * \param wFileName：PE文件名
 */
PortableExecutable::PortableExecutable(WCHAR *wFileName)
{
    // PE文件句柄
    HANDLE hFile = NULL;
    // PE文件大小
    DWORD dwFileSize = 0;
    // 载入后内存基址
    DWORD64 dwBaseAddress = NULL;
    // 内存偏移
    DWORD64 dwOffset = 0;
    // 存HRESULT型返回值
    HRESULT hr = S_OK;
    // 存BOOL型返回值
    BOOL bRet = TRUE;

    // 打开PE文件
    hFile = ::CreateFile(wFileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }
    // 申请内存存放PE文件
    dwFileSize = ::GetFileSize(hFile, NULL);
    if (dwFileSize == NULL)
    {
        return;
    }
    dwBaseAddress = (DWORD64)::VirtualAlloc(NULL, dwFileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (dwBaseAddress == NULL)
    {
        return;
    }
    // 将PE文件载入内存
    ::SetFilePointer(hFile, NULL, NULL, FILE_BEGIN);
    bRet = ::ReadFile(hFile, (LPVOID)dwBaseAddress, dwFileSize, NULL, NULL);
    if (!bRet)
    {
        ::CloseHandle(hFile);
        return;
    }
    // 关闭PE文件
    ::CloseHandle(hFile);

    // 载入NT头
    while (dwOffset <= dwFileSize)
    {
        if (*(DWORD *)(dwBaseAddress + dwOffset) == NTSIGNATURE)
        {
            ntHeader32 = *(IMAGE_NT_HEADERS32 *)(dwBaseAddress + dwOffset);
            break;
        }
        dwOffset += 4;
    }
    if (ntHeader32.Signature != NTSIGNATURE)
    {
        return;
    }
    dwOffset += sizeof(IMAGE_NT_HEADERS32);

    // 载入节表
    sectionHeaderList = new IMAGE_SECTION_HEADER[ntHeader32.FileHeader.NumberOfSections]();
    for (DWORD i = 0; i < ntHeader32.FileHeader.NumberOfSections; i++)
    {
        sectionHeaderList[i] = *(IMAGE_SECTION_HEADER *)(dwBaseAddress + dwOffset);
        dwOffset += sizeof(IMAGE_SECTION_HEADER);
    }

    // 载入导出表
    this->LoadExportFunctions(dwBaseAddress);

    // 释放内存
    ::VirtualFree((LPVOID)dwBaseAddress, 0, MEM_RELEASE);
}

PortableExecutable::~PortableExecutable()
{
    if (sectionHeaderList != NULL)
    {
        delete[] sectionHeaderList;
    }
    if (exportFunctionList != NULL)
    {
        delete[] exportFunctionList;
    }
}

/**
 * 根据函数名获取导出函数地址.
 *
 * \param functionName：函数名（部分）
 * \return 非0：导出函数地址 0：未找到
 */
DWORD PortableExecutable::GetFunctionAddressByName(const CHAR *functionName)
{
    // 遍历导出函数表
    for (DWORD i = 0; i < exportDirectory.NumberOfFunctions; i++)
    {
        // 跳过无名函数
        if (strlen(exportFunctionList[i].functionName) == 0)
        {
            continue;
        }
        if (strncmp(functionName, exportFunctionList[i].functionName, strlen(functionName)) == 0)
        {
            return exportFunctionList[i].dwFunctionAddress;
        }
    }
    return 0;
}
