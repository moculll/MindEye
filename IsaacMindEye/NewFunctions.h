#pragma once
#include "Common.h"

namespace NewFunctions
{
	// CreateProcessA函数
	typedef BOOL(WINAPI *CREATEPROCESSA)(
		LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
		LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
		LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
	extern CREATEPROCESSA fpCreateProcessA;
	BOOL WINAPI CreateProcessA(
		LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
		LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
		LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

	// CreateThread函数
	typedef HANDLE (*CREATETHREAD)(
		LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
	extern CREATETHREAD fpCreateThread;
	HANDLE CreateThread(
		LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);

	// SwapBuffers函数
	typedef BOOL(WINAPI *SWAPBUFFERS)(HDC unnamedParam1);
	extern SWAPBUFFERS fpSwapBuffers;
	BOOL WINAPI SwapBuffers(HDC unnamedParam1);

	// glDrawElements函数
	typedef void(GLAPIENTRY *GLDRAWELEMENTS)(GLenum mode, GLsizei count, GLenum type, const void *indices);
	extern GLDRAWELEMENTS fpGlDrawElements;
	void GLAPIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);

	// glBindTexture函数
	typedef void(GLAPIENTRY *GLBINDTEXTURE)(GLenum target, GLuint texture);
	extern GLBINDTEXTURE fpGlBindTexture;
	void GLAPIENTRY glBindTexture(GLenum target, GLuint texture);
}
