//#define _CRT_SECURE_NO_WARNINGS
#include "..\common\last_error.h"
#include "..\common\create_child.h"

#include <iostream>
#include <string>
#include <regex>
#include <cctype>
#include <cstdlib>
#include <cassert>

#include <windows.h>

constexpr char child_process_cmd[] = "cmd.exe /K echo cmd.exe is running";
constexpr size_t max_command_len = 4096;
const std::string blacklist[] = { "^ls", "^uname", "bla-bla" };

bool write_to_handle(HANDLE h, std::string& str, std::regex& rx_black)
{
	if (std::regex_search(str, rx_black))
	{
		std::cout << MSG_PREFIX "Command in the blacklist: " << str << std::endl;
		return true;
	}
	std::cout << MSG_PREFIX "Send to cmd: \"" << str << "\"" << std::endl;
	str += "\n";
	DWORD count = 0;
	if (WriteFile(h, str.c_str(), static_cast<DWORD>(str.length()), &count, NULL))
	{
		return true;
	}
	last_error_msg(TEXT(__FUNCTION__ ".WriteFile"));
	return false;
}

std::regex create_regex()
{
	std::string rstr;
	for (auto i = std::cbegin(blacklist); i < std::cend(blacklist); ++i)
	{
		rstr += "("; rstr += *i; rstr += ")|";
	}
	assert(!rstr.empty());
	rstr.pop_back();
	std::cout << MSG_PREFIX "complete regex: " << rstr << std::endl;
	return std::move(std::regex{ rstr, std::regex_constants::icase });
}

int getch(HANDLE hchild, HANDLE hin, HANDLE hin_ev)
{
	OVERLAPPED ov{ 0 };
	DWORD count = 0;
	char ch;
	ov.hEvent = hin_ev;
	ResetEvent(hin_ev);
	if (ReadFile(hin, &ch, 1, &count, &ov))
	{
		return ch;
	}
	const DWORD err = GetLastError();
	if (err != ERROR_IO_PENDING)
	{
		last_error_msg(TEXT(__FUNCTION__ ".ReadFile"), err);
		return -1;
	}

	HANDLE h[] = { hchild, hin_ev };
	while(true)
	{
		const DWORD rc = WaitForMultipleObjects(sizeof(h)/sizeof(h[0]), h, false, INFINITE);
		if (rc == WAIT_FAILED)
		{
			last_error_msg(TEXT(__FUNCTION__ ".WaitForMultipleObjects"));
			break;
		}
		if ((rc - WAIT_OBJECT_0) == 0)
		{
			std::cout << MSG_PREFIX " child process is finished" << std::endl;
			break;
		}
		assert((rc - WAIT_OBJECT_0) == 1);

		const bool ret = GetOverlappedResult(hin, &ov, &count, false);
		if (ret && count == 1)
		{
			return ch;
		}
		last_error_msg(TEXT(__FUNCTION__ ".GetOverlappedResult"));
		break;

	}

	return -1;
}

int main(int argc, char *argv[])
{
	const HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
	const HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	const HANDLE hstderr = GetStdHandle(STD_ERROR_HANDLE);
	std::cout << MSG_PREFIX "is starting!" << std::endl;
	std::cout << MSG_PREFIX "stdin handle: 0x" << hstdin
		<< "\tstdout handle: 0x" << hstdout
		<< "\tstderr handle: 0x" << hstderr << std::endl;
	HANDLE hin;
	if (argc == 2)
	{
		char *end = argv[1];
		hin = reinterpret_cast<HANDLE>(std::strtoll(argv[1], &end, 0));
		if (*end != 0)
		{
			last_error_msg(TEXT("Invalid argument"));
			return 1;
		}
	} else {
		last_error_msg(TEXT("Invalid argument"));
		return 1;
	}
	std::cout << MSG_PREFIX "input handle: 0x" << hin << std::endl;

	std::regex rx = create_regex();

	const HANDLE hin_ev = CreateEvent(
		NULL,		// default security attribute 
		TRUE,		// manual-reset event 
		FALSE,		// initial state = nonsignaled 
		NULL);		// unnamed event object
	if (hin_ev == NULL)
	{
		last_error_msg(TEXT(__FUNCTION__ ".CreateEvent"));
		return 1;
	}

	child_handle_s child_h = create_child_process(child_process_cmd);
	if (child_h.in_write == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	std::string line;
	while(check_child(child_h.hproc))
	{
		const int chi = getch(child_h.hproc, hin, hin_ev);
		if (chi == -1)
		{
			break;
		} else if (chi == 0)
		{
			continue;
		}

		char ch = static_cast<char>(chi);

		if (std::isalnum(ch))
		{
			std::cout << MSG_PREFIX "intput char: " << ch << std::endl;
		} else
		{
			std::cout << MSG_PREFIX "intput char code: " << chi << std::endl;
		}

		if (!check_child(child_h.hproc))
		{
			break;
		}

		if (ch != '\r')
		{
			line += ch;
			if (line.length() >= max_command_len)
			{
				std::cout << MSG_PREFIX "Command line is very big. Max length is: " 
					<< max_command_len << std::endl;
			}
		} else {		// end of line
			size_t line_len = line.length();
			if (line.length() > 0)
			{
				if (!write_to_handle(child_h.in_write, line, rx))
				{
					break;
				}
				line.clear();
			}
		}
	}

	CloseHandle(child_h.hproc);
	CloseHandle(child_h.in_write);
	CloseHandle(hin);
	CloseHandle(hin_ev);
	std::cout << MSG_PREFIX "Goodbye!" << std::endl;
}
