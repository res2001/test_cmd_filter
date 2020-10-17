#include "pipeex.h"
#include "..\common\create_child.h"
#include "..\common\last_error.h"

#include <iostream>
#include <string>
#include <cctype>
#include <cassert>

#include <windows.h>

constexpr char child_process_cmd[] = "filter.exe";

bool create_pipes_overlapped(pipes_handle_s& h)
{
	h = pipes_handle_s{ INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
	SECURITY_ATTRIBUTES sa = { 0 };
	// Set the bInheritHandle flag so pipe handles are inherited. 
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	bool ret = false;
	do
	{
		// Create a pipe for the child process's
		if (!MyCreatePipePw(&h.in_read, &h.in_write, &sa, 0, FILE_FLAG_OVERLAPPED, 0, PIPE_NOWAIT))
		{
			last_error_msg(TEXT(__FUNCTION__ ".CreatePipeEx"));
			break;
		}
		std::cout << MSG_PREFIX "Create named pipe: read side: 0x" << h.in_read
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

int getch(HANDLE hin, HANDLE hchild)
{
	HANDLE h[2] = { hchild, hin };
	INPUT_RECORD inrec{ 0 };
	do {
		const DWORD rc = WaitForMultipleObjects(2, h, false, INFINITE);
		if (rc == WAIT_FAILED)
		{
			last_error_msg(TEXT(__FUNCTION__ ".WaitForMultipleObjects"));
			return -1;
		}
		if ((rc - WAIT_OBJECT_0) == 0)
		{
			std::cout << MSG_PREFIX " child process is finished" << std::endl;
			return -1;
		}

		DWORD        count = 0;
		if (!ReadConsoleInput(hin, &inrec, 1, &count))
		{
			break;
		}

		if (count == 0)
		{
			last_error_msg(TEXT(__FUNCTION__ ".ReadConsoleInput"));
			return -1;
		}

	} while (!(inrec.EventType == KEY_EVENT && inrec.Event.KeyEvent.bKeyDown /*&& inrec.Event.KeyEvent.uChar.AsciiChar != '\r'*/));

	return inrec.Event.KeyEvent.uChar.AsciiChar;
}

int main()
{
	const HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
	const HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
	const HANDLE herr = GetStdHandle(STD_ERROR_HANDLE);
	std::cout << MSG_PREFIX "is starting!" << std::endl;
	std::cout << MSG_PREFIX "stdin handle: 0x" << hin
		<< "\tstdout handle: 0x" << hout
		<< "\tstderr handle: 0x" << herr << std::endl;

	child_handle_s child_h;
	{
		struct pipes_handle_s h;
		if (!create_pipes_overlapped(h))
		{
			return 1;
		}

		std::string cmd_str(child_process_cmd);
		cmd_str += " "; 
		cmd_str += std::to_string(reinterpret_cast<size_t>(h.in_read));

		HANDLE child_in_read = h.in_read;
		h.in_read = INVALID_HANDLE_VALUE;
		child_h = create_child_process(cmd_str.c_str(), h);
		if (child_h.in_write == INVALID_HANDLE_VALUE)
		{
			h.in_read = child_in_read;
			close_pipes(h);
			return 1;
		}
		CloseHandle(child_in_read);
	}

	FlushConsoleInputBuffer(hin);

	while (true)
	{
		const int chi = getch(hin, child_h.hproc);
		if (chi == -1)
		{
			break;
		} else if (chi == 0) {
			continue;
		}

		char ch = static_cast<char>(chi);

		if (std::isalnum(ch))
		{
			std::cout << MSG_PREFIX "intput char: " << ch << std::endl;
		} else {
			std::cout << MSG_PREFIX "intput char code: " << chi << std::endl;
		}

		if (!check_child(child_h.hproc))
		{
			break;
		}

		DWORD count = 0;
		if (!WriteFile(child_h.in_write, &ch, 1, &count, NULL))
		{
			last_error_msg(TEXT(__FUNCTION__ ".WriteFile"));
			break;
		}
	}

	CloseHandle(child_h.hproc);
	CloseHandle(child_h.in_write);
	std::cout << MSG_PREFIX "Goodbye!" << std::endl;
}
