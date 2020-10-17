#pragma once
#include <windows.h>

void error_msg(const TCHAR *str);
void last_error_msg(LPCTSTR func, int err);
void last_error_msg(LPCTSTR func, DWORD err);
DWORD last_error_msg(LPCTSTR func);
