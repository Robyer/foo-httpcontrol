#ifndef __FOO_HTTPCONTROL_H__
#define __FOO_HTTPCONTROL_H__

#define _CRT_SECURE_NO_DEPRECATE 1

#include <map>
#include "ui.h"
#include "httpserver.h"

#define WM_HTTPSERVER_CMD	0x8899
#define MAX_THREADS			8

namespace httpc
{
namespace control
{
	typedef std::map<pfc::string8, bool (* )(foo_httpserver_command *)> commands_map;
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

class process_locations_notify_my : public process_locations_notify { // callback from enqueue call
public:
	void on_completion(const pfc::list_base_const_t<metadb_handle_ptr> & p_items)
	{
		static_api_ptr_t<playlist_manager> plm;

		if (httpc::extensions.get_count() > 0)
		{
			list_t<metadb_handle_ptr> p_items_toadd;

			t_size l = p_items.get_count();

			for (size_t i = 0; i < l; ++i)
				if (httpc::is_extension_registered(p_items[i]->get_path()) != -1)
					p_items_toadd.add_item(p_items[i]);

			plm->activeplaylist_add_items(p_items_toadd, bit_array_true());
		}
		else
		{
			plm->activeplaylist_add_items(p_items, bit_array_true());
		}

		on_aborted();
	};

	void on_aborted() { httpc::enqueueing = false; };
};

#endif /*__FOO_HTTPCONTROL_H__*/