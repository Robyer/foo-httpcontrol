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
	extern ATOM	g_class_atom;
	extern HANDLE	m_io_thread;
	extern HWND	m_hwnd;
	extern HANDLE listener_stop_event;
	extern HANDLE listener_init_event;
	extern HANDLE process_command_event;
	extern volatile bool listener_started;
	extern volatile bool listener_error;
	extern volatile bool listener_stop;
	extern volatile int running_threads;
	extern pfc::string8 listener_info;

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