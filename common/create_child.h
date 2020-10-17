#pragma once
#include <windows.h>

struct pipes_handle_s {
	HANDLE in_read, in_write, out_write, err_write;
};

struct child_handle_s {
	HANDLE in_write, hproc;
};

void close_pipes(pipes_handle_s& h);
bool create_pipes(pipes_handle_s& h);
child_handle_s create_child_process(const char *cmd, pipes_handle_s& h);
child_handle_s create_child_process(const char *cmd);
bool check_child(HANDLE h);

