#include "stdafx.h"
#include "httpcontrol.h"

DECLARE_COMPONENT_VERSION(
	"HTTP Control",
	"0.97.15",
	"control foobar2000 via http "__DATE__)
	
VALIDATE_COMPONENT_FILENAME("foo_httpcontrol.dll");

namespace httpc 
{
namespace control 
{
	ATOM g_class_atom;
	pfc::string8 class_name;
	pfc::stringcvt::string_os_from_utf8 os_class_name;
	HANDLE	m_io_thread;
	HWND	m_hwnd;
	HANDLE	listener_stop_event;
	HANDLE	listener_init_event;
	HANDLE process_command_event;
	volatile bool listener_started;
	volatile bool listener_error;
	volatile bool listener_stop;
	volatile int running_threads;
	pfc::string8 listener_info;

	void process_command(pfc::string_base &cmd, pfc::string_base &param1, pfc::string_base &param2, pfc::string_base &param3)
	{
		if (httpc::control::is_active())
			uSendMessage(httpc::control::m_hwnd, WM_HTTPSERVER_CMD, 0, (LPARAM) (new foo_httpserver_command(cmd, param1, param2, param3)) );
	}

	unsigned long WINAPI foo_process_request_thread(LPVOID param)
	{
		++running_threads;
		foo_httpserv *httpserv = reinterpret_cast<foo_httpserv *>(param);
		httpserv->process_request();
		--running_threads;
		delete httpserv;
		return 0;
	}

	void set_active(bool activate)
	{
		listener_info.reset();

		if(activate) // start listener
		{
			WNDCLASS wc;
			memset(&wc,0,sizeof(wc));
			wc.style = 0;
			wc.lpfnWndProc = wndproc;
			wc.hInstance = core_api::get_my_instance();
			class_name = pfc::string8(foo_name) << pfc::string8("_class_") << pfc::format_hex((t_uint64)GetTickCount());
			os_class_name = pfc::stringcvt::string_os_from_utf8(class_name);
			wc.lpszClassName = os_class_name;
			g_class_atom = RegisterClass(&wc);

			m_hwnd = uCreateWindowEx(0,
									class_name,
									foo_name,
									0,
									CW_USEDEFAULT,
									CW_USEDEFAULT,
									0,0,
									core_api::get_main_window(),
									0,
									core_api::get_my_instance(),
									0);
			if(!m_hwnd) 
				foo_error("couldn't create helper window");
			else
			{
				listener_stop = false;
				listener_init_event = CreateEvent(NULL, FALSE, FALSE, NULL);

				m_io_thread = CreateThread(NULL,0,http_listener,NULL,0,NULL);

				WaitForSingleObject(listener_init_event,5000);

				CloseHandle(listener_init_event);

				if(m_io_thread == NULL || ! listener_started || listener_error)
					foo_error("couldn't create helper thread");
			}
		} else // stop listener
		{
			if(m_io_thread != NULL)
			{
				listener_stop = true;
				SetEvent(listener_stop_event);
				CloseHandle(m_io_thread);
			}

			if(m_hwnd != NULL)
			{
				uDestroyWindow(m_hwnd);
				m_hwnd = NULL;
			}

			if (g_class_atom)
				UnregisterClassW(os_class_name, core_api::get_my_instance());
		}
	}

	bool is_active()
	{
		return listener_started;
	}

	unsigned long WINAPI http_listener(LPVOID param)
	{	
		DWORD ret;

		JNL::open_socketlib();
		JNL_Listen l((short)cfg.main.port, cfg.main.ip != ""? inet_addr(cfg.main.ip) : 0);

		listener_error = true;
		listener_started = false;

		if (l.is_error())
			foo_error("bind failed.");
		else
		{
			listener_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);

			HANDLE handles[2];
			handles[0] = listener_stop_event;					// stop listening
			handles[1] = WSACreateEvent();						// network activity

			if (handles[1] == WSA_INVALID_EVENT)
				foo_error("WSACreateEvent failed.");
			else
			{
				if (WSAEventSelect(l.m_socket, handles[1], FD_ACCEPT/* | FD_CONNECT | FD_READ | FD_CLOSE*/) != 0)
					foo_error("WSAEventSelect failed. Make sure IP address specified in \"Listen on\" component preferences field is valid and Port:IP combination is not used by another process.");
				else
				{
					listener_started = true;
					listener_error = false;

					if (strcmp(cfg.main.ip, "0.0.0.0") == 0)
						listener_info = pfc::string_formatter() << "http://127.0.0.1:" << cfg.main.port;
					else
						listener_info = pfc::string_formatter() << "http://" << cfg.main.ip << ":" << cfg.main.port;

					SetEvent(listener_init_event);

					while (!l.is_error())
					{
						ret = WSAWaitForMultipleEvents(2, handles, FALSE, pfc::infinite_size, TRUE);

						WSAResetEvent(handles[1]); // assuming network activity event occured

						if(listener_stop || ret == WSA_INVALID_HANDLE)
						{
							if (ret == WSA_INVALID_HANDLE)
								foo_error("WSAWaitForMultipleEvents got invalid handle, stopping listener thread.");

							break;
						}

						if (ret != WSA_WAIT_TIMEOUT && ret != WSA_WAIT_IO_COMPLETION)
						{
							JNL_Connection *con;

							while (!listener_stop && running_threads >= MAX_THREADS)
								Sleep(1);

							while ((con = l.get_connect(SEND_BUFFER, RECIEVE_BUFFER)) && !listener_stop)
							if (con)
							{
								foo_httpserv *httpserv = new foo_httpserv(con);

								HANDLE m_worker_thread = 
									CreateThread(NULL,0,foo_process_request_thread,reinterpret_cast<void *>(httpserv),0,NULL);

								if (m_worker_thread == NULL)
								{
									delete httpserv;
									foo_error("cannot spawn request processing thread");
								}
								else
									CloseHandle(m_worker_thread);							
							}
							else
								break;

						}
					}
				}
			}

			CloseHandle(listener_stop_event);
			WSACloseEvent(handles[1]);

			listener_started = false;
		}

		SetEvent(listener_init_event);

		JNL::close_socketlib();

		return 0;
	}

	bool execute_command (commands_map::iterator &cmd_it, foo_httpserver_command *cmd)
	{
		__try
		{
			return (* cmd_it->m_value)(cmd);
		}
		__except_instacrash {}

		return false;
	}

	LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
			case WM_HTTPSERVER_CMD:
			{
				bool config_changed = tcfg.is_changed();

				if (config_changed)
					httpc::playlist_page = pfc::infinite_size;

				foo_httpserver_command *cmd = (foo_httpserver_command*)lParam;

				if (cmd)
				{
					pfc::string8 &command = cmd->get_command();

					httpc::last_action = httpc::FLC_START;
					httpc::sac = static_api_ptr_t<playback_control>()->get_stop_after_current();
					httpc::refresh_volume(1);

					commands_map::iterator cmd_it = commands.find(command);

					if (cmd_it != NULL)
					{
						bool result = execute_command(cmd_it, cmd);

						if (!result)
							foo_error(string_formatter() << "command " << command << " failed");
					}
					else
						if (command.get_length())
							foo_error(string_formatter() << "unknown command: " << command);

					delete cmd;
				}

				if (config_changed)
				{
					httpc::titleformat_compile();

					if (httpc::playlist_page == pfc::infinite_size)
						httpc::should_focus_on_playing = true;
				}

				if (should_update_playlist 
					|| config_changed
					|| httpc::pb_item == pfc::infinite_size
					|| (httpc::active_playlist == httpc::pb_playlist && httpc::playlist_page_switched))
				{
					httpc::refresh_playing_info();
					httpc::refresh_playlist_view();
					httpc::refresh_playback_queue();
				}

				if (should_update_playlist_total_time
					|| config_changed)
					httpc::refresh_playlist_total_time();

				if (httpc::should_update_queue)
					httpc::refresh_playback_queue();

				if (httpc::should_update_playlists_list)
					httpc::refresh_playlist_list();

				if (httpc::control::process_command_event)
					SetEvent(httpc::control::process_command_event);
			}
			break;
			default:
			break;
		}

		return uDefWindowProc(hwnd,msg,wParam,lParam);
	}

}
}
