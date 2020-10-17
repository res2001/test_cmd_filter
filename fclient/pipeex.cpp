
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples.
*       Copyright 1995 - 1997 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the
*       Microsoft samples programs.
\******************************************************************************/

/*++

Copyright (c) 1997  Microsoft Corporation

Module Name:

	pipeex.c

Abstract:

	CreatePipe-like function that lets one or both handles be overlapped

Author:

	Dave Hart  Summer 1997

Revision History:

--*/
#include "..\common\last_error.h"
#include <cstdio>
#include <windows.h>

static ULONG PipeSerialNumber;

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
	)

/*++

Routine Description:

	The CreatePipeEx API is used to create an anonymous pipe I/O device.
	Unlike CreatePipe FILE_FLAG_OVERLAPPED may be specified for one or
	both handles.
	Two handles to the device are created.  One handle is opened for
	reading and the other is opened for writing.  These handles may be
	used in subsequent calls to ReadFile and WriteFile to transmit data
	through the pipe.

Arguments:

	lpReadPipe - Returns a handle to the read side of the pipe.  Data
		may be read from the pipe by specifying this handle value in a
		subsequent call to ReadFile.

	lpWritePipe - Returns a handle to the write side of the pipe.  Data
		may be written to the pipe by specifying this handle value in a
		subsequent call to WriteFile.

	lpPipeAttributes - An optional parameter that may be used to specify
		the attributes of the new pipe.  If the parameter is not
		specified, then the pipe is created without a security
		descriptor, and the resulting handles are not inherited on
		process creation.  Otherwise, the optional security attributes
		are used on the pipe, and the inherit handles flag effects both
		pipe handles.

	nSize - Supplies the requested buffer size for the pipe.  This is
		only a suggestion and is used by the operating system to
		calculate an appropriate buffering mechanism.  A value of zero
		indicates that the system is to choose the default buffering
		scheme.

Return Value:

	TRUE - The operation was successful.

	FALSE/NULL - The operation failed. Extended error status is available
		using GetLastError.

--*/

{
	HANDLE ReadPipeHandle, WritePipeHandle;
	DWORD dwError;
	char PipeNameBuffer[ MAX_PATH ];

	//
	// Only one valid OpenMode flag - FILE_FLAG_OVERLAPPED
	//

	if ((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	//
	//  Set the default timeout to 120 seconds
	//

	if (nSize == 0) {
		nSize = 4096;
		}

	std::snprintf( PipeNameBuffer, sizeof(PipeNameBuffer),
			 "\\\\.\\Pipe\\RemoteExeAnon.%08x.%08x",
			 GetCurrentProcessId(),
			 InterlockedIncrement(&PipeSerialNumber)
		   );

	WritePipeHandle = CreateNamedPipeA(
						 PipeNameBuffer,
						 PIPE_ACCESS_OUTBOUND | dwWriteMode,
						 PIPE_TYPE_BYTE | dwPipeMode,
						 1,             // Number of pipes
						 nSize,         // Out buffer size
						 nSize,         // In buffer size
						 120 * 1000,    // Timeout in ms
						 lpPipeAttributes
						 );

	if (!WritePipeHandle) {
		return FALSE;
	}

	ReadPipeHandle = CreateFileA(
						PipeNameBuffer,
						GENERIC_READ,
						0,                         // No sharing
						lpPipeAttributes,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL | dwReadMode,
						NULL                       // Template file
					  );

	if (INVALID_HANDLE_VALUE == ReadPipeHandle) {
		dwError = GetLastError();
		CloseHandle( ReadPipeHandle );
		SetLastError(dwError);
		return FALSE;
	}

	*lpReadPipe = ReadPipeHandle;
	*lpWritePipe = WritePipeHandle;
	return( TRUE );
}

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
)

/*++

Routine Description:

	The CreatePipeEx API is used to create an anonymous pipe I/O device.
	Unlike CreatePipe FILE_FLAG_OVERLAPPED may be specified for one or
	both handles.
	Two handles to the device are created.  One handle is opened for
	reading and the other is opened for writing.  These handles may be
	used in subsequent calls to ReadFile and WriteFile to transmit data
	through the pipe.

Arguments:

	lpReadPipe - Returns a handle to the read side of the pipe.  Data
		may be read from the pipe by specifying this handle value in a
		subsequent call to ReadFile.

	lpWritePipe - Returns a handle to the write side of the pipe.  Data
		may be written to the pipe by specifying this handle value in a
		subsequent call to WriteFile.

	lpPipeAttributes - An optional parameter that may be used to specify
		the attributes of the new pipe.  If the parameter is not
		specified, then the pipe is created without a security
		descriptor, and the resulting handles are not inherited on
		process creation.  Otherwise, the optional security attributes
		are used on the pipe, and the inherit handles flag effects both
		pipe handles.

	nSize - Supplies the requested buffer size for the pipe.  This is
		only a suggestion and is used by the operating system to
		calculate an appropriate buffering mechanism.  A value of zero
		indicates that the system is to choose the default buffering
		scheme.

Return Value:

	TRUE - The operation was successful.

	FALSE/NULL - The operation failed. Extended error status is available
		using GetLastError.

--*/

{
	HANDLE ReadPipeHandle, WritePipeHandle;
	DWORD dwError;
	char PipeNameBuffer[MAX_PATH];

	//
	// Only one valid OpenMode flag - FILE_FLAG_OVERLAPPED
	//

	if ((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	//
	//  Set the default timeout to 120 seconds
	//

	if (nSize == 0)
	{
		nSize = 4096;
	}

	std::snprintf(PipeNameBuffer, sizeof(PipeNameBuffer),
		"\\\\.\\Pipe\\RemoteExeAnon.%08x.%08x",
		GetCurrentProcessId(),
		InterlockedIncrement(&PipeSerialNumber)
	);

	WritePipeHandle = CreateNamedPipeA(
		PipeNameBuffer,
		PIPE_ACCESS_OUTBOUND | dwWriteMode,
		PIPE_TYPE_BYTE | dwPipeMode,
		1,             // Number of pipes
		nSize,         // Out buffer size
		nSize,         // In buffer size
		120 * 1000,    // Timeout in ms
		lpPipeAttributes
	);

	if (!WritePipeHandle)
	{
		return FALSE;
	}

	ReadPipeHandle = CreateFileA(
		PipeNameBuffer,
		GENERIC_READ,
		0,                         // No sharing
		lpPipeAttributes,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | dwReadMode,
		NULL                       // Template file
	);

	if (INVALID_HANDLE_VALUE == ReadPipeHandle)
	{
		dwError = GetLastError();
		CloseHandle(ReadPipeHandle);
		SetLastError(dwError);
		return FALSE;
	}

	*lpReadPipe = ReadPipeHandle;
	*lpWritePipe = WritePipeHandle;
	return(TRUE);
}

int WriteToPipe(HANDLE fh, char *str)
{
	DWORD dwWritten;
	BOOL bSuccess = FALSE;

	bSuccess = WriteFile(fh, str, (DWORD)strlen(str), &dwWritten, NULL);
	if (!bSuccess) {
		last_error_msg(TEXT(__FUNCTION__ ".WriteFile"));
	}
	return (int)bSuccess;
}

int ReadFromPipe(HANDLE fh, char *str, DWORD slen)
{
	DWORD dwRead;
	BOOL bSuccess = FALSE;

	bSuccess = ReadFile(fh, str, slen, &dwRead, NULL);

	return bSuccess ? dwRead : -1;
}
