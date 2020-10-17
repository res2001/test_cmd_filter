#include "create_child.h"
#include "last_error.h"

#include <iostream>
#include <string>

void close_pipes(pipes_handle_s& h)
{
	CloseHandle(h.in_read);
	CloseHandle(h.in_write);
	h.in_read = INVALID_HANDLE_VALUE;
	h.in_write = INVALID_HANDLE_VALUE;
}

bool create_pipes(pipes_handle_s& h)
{
	h = pipes_handle_s{ INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
	SECURITY_ATTRIBUTES sa = { 0 };
	// Set the bInheritHandle flag so pipe handles are inherited. 
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	bool ret = false;
	do
	{
		if (!CreatePipe(&h.in_read, &h.in_write, &sa, 0))
		{
			last_error_msg(TEXT(__FUNCTION__ ".CreatePipe"));
			break;
		}
		std::cout << MSG_PREFIX "Create unnamed pipe: read side: 0x" << h.in_read 
			<< "\twrite side: 0x" << h.in_write << std::endl;

		h.out_write = GetStdHandle(STD_OUTPUT_HANDLE);
		h.err_write = GetStdHandle(STD_ERROR_HANDLE);
		if (!SetHandleInformation(h.in_write, HANDLE_FLAG_INHERIT, 0)	// no inheritanse
			|| !SetHandleInformation(h.out_write, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)	// set inheritance
			|| !SetHandleInformation(h.err_write, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))	// set inheritance
		{
			last_error_msg(TEXT(__FUNCTION__ ".SetHandleInformation"));
			break;
		}

		ret = true;
	} while (0);

	if (!ret)
		close_pipes(h);

	return ret;
}

child_handle_s create_child_process(const char *cmd, pipes_handle_s& h)
{
	std::string szCmdline(cmd);
	PROCESS_INFORMATION piProcInfo = { 0 };
	STARTUPINFOA siStartInfo = { 0 };

	siStartInfo.cb = sizeof(siStartInfo);
	siStartInfo.hStdError = h.err_write;
	siStartInfo.hStdOutput = h.out_write;
	siStartInfo.hStdInput = h.in_read;
	siStartInfo.dwFlags = STARTF_USESTDHANDLES;

	// Create the child process. 
	const BOOL bSuccess = CreateProcessA(NULL,		// application name
		const_cast<char*>(szCmdline.c_str()),		// command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags: DETACHED_PROCESS | CREATE_NEW_CONSOLE, CREATE_NO_WINDOW | CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	 // If an error occurs, exit the application. 
	if (!bSuccess)
	{
		last_error_msg(TEXT(__FUNCTION__ ".CreateProcess"));
		return child_handle_s{ INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
	}
	// Close handles to the child process and its primary thread.
	// Some applications might keep these handles to monitor the status
	// of the child process, for example. 

	CloseHandle(piProcInfo.hThread);

	// Close handles to the stdin and stdout pipes no longer needed by the child process.
	// If they are not explicitly closed, there is no way to recognize that the child process has ended.

	CloseHandle(h.in_read);
	h.in_read = INVALID_HANDLE_VALUE;
	Sleep(500);
	return child_handle_s{ h.in_write, piProcInfo.hProcess };
}

child_handle_s create_child_process(const char *cmd)
{
	pipes_handle_s h;
	if (!create_pipes(h))
	{
		close_pipes(h);
		return child_handle_s{ INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
	}
	return create_child_process(cmd, h);
}

bool check_child(HANDLE h)
{
	if (WaitForSingleObject(h, 0) == WAIT_TIMEOUT)
	{
		return true;
	}
	std::cout << MSG_PREFIX " child process is finished" << std::endl;
	return false;
}
