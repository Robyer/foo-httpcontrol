#ifndef __FOO_HTTPCONTROL_H__
#define __FOO_HTTPCONTROL_H__

#define _CRT_SECURE_NO_DEPRECATE 1

#include "ui.h"
#include "httpserver.h"

#define WM_HTTPSERVER_CMD	0x8899
#define MAX_THREADS			8

namespace httpc
{
namespace control
{
	extern commands_map commands;

	extern void gen_cmd_table();

	extern void set_active(bool activate);
	extern bool is_active();

	static unsigned long WINAPI read_command_thread(LPVOID param);
    static unsigned long WINAPI http_listener(LPVOID param);
	static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}
}

#endif /*__FOO_HTTPCONTROL_H__*/