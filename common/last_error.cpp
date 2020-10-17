#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>

void error_msg(const TCHAR *str)
{
	std::wcerr << TEXT(MSG_PREFIX) << str << std::endl;
}

void last_error_msg(LPCTSTR func, int err)
{
	std::wcerr << TEXT(MSG_PREFIX) << func << L"() error " << err << L": " << _wcserror(err) << std::endl;
}

void last_error_msg(LPCTSTR func, DWORD err)
{
	LPTSTR lpMsgBuf = NULL;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	std::wcerr << TEXT(MSG_PREFIX) << func << L"() error " << err << L": " << lpMsgBuf << std::endl;

	LocalFree(lpMsgBuf);
}

DWORD last_error_msg(LPCTSTR func)
{
	const DWORD err = GetLastError();
	last_error_msg(func, err);
	return err;
}
