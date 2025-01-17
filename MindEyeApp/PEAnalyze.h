#pragma once
#include "Common.h"

DWORD RVA2FOA(DWORD dwRVA, IMAGE_SECTION_HEADER *sectionHeaderList, DWORD dwSectionCount);

struct ExportFunction
{
	DWORD64 dwFunctionAddress;
	CHAR functionName[MAX_PATH];
};

class PortableExecutable
{
private:
	// 函数

	VOID LoadExportFunctions(DWORD64 dwBaseAddress);
	DWORD RVA2FOA(DWORD dwRVA);

protected:
	// 常量
	/** NT头标志 */
	static const DWORD NTSIGNATURE = 0x00004550;

public:
	PortableExecutable(WCHAR *wFileName);
	~PortableExecutable();

	// 变量
	/** NT头 */
	IMAGE_NT_HEADERS32 ntHeader32 = { 0 };
	/** 节表 */
	IMAGE_SECTION_HEADER* sectionHeaderList = NULL;
	/** 导出表索引 */
	IMAGE_EXPORT_DIRECTORY exportDirectory = { 0 };
	/** 导出表 */
	ExportFunction* exportFunctionList = NULL;

	// 函数

	DWORD GetFunctionAddressByName(const CHAR *functionName);
};
