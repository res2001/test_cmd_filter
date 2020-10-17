#pragma once
#include <windows.h>

BOOL
APIENTRY
MyCreatePipePw(
	OUT LPHANDLE lpReadPipe,
	OUT LPHANDLE lpWritePipe,
	IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
	IN DWORD nSize,
	DWORD dwReadMode,
	DWORD dwWriteMode,
	DWORD dwPipeMode
);
BOOL
APIENTRY
MyCreatePipePr(
	OUT LPHANDLE lpReadPipe,
	OUT LPHANDLE lpWritePipe,
	IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
	IN DWORD nSize,
	DWORD dwReadMode,
	DWORD dwWriteMode,
	DWORD dwPipeMode
);
