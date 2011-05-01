#include "stdafx.h"
#include "httpcontrol.h"

DECLARE_COMPONENT_VERSION(
	"HTTP Control",
	"0.97.7",
	"control foobar2000 via http "__DATE__)
	
VALIDATE_COMPONENT_FILENAME("foo_httpcontrol.dll");

namespace httpc 
{
namespace control 
{
	ATOM g_class_atom;
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

	commands_map commands;

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
		pfc::stringcvt::string_os_from_utf8 os_class_name(pfc::string8("CLS_foo_httpcontrol_") << pfc::format_hex((t_uint64)GetTickCount()));
		wc.lpszClassName = os_class_name;
		g_class_atom = RegisterClass(&wc);

		m_hwnd = uCreateWindowEx(0,
								(const char *)g_class_atom,
								"foo_httpcontrol",
								0,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								0,0,
								core_api::get_main_window(),
								0,
								core_api::get_my_instance(),
								0);
		if(!m_hwnd) 
			foo_info("error: couldn't create helper window.");
		else
		{
			listener_stop = false;
			listener_init_event = CreateEvent(NULL, FALSE, FALSE, NULL);

			m_io_thread = CreateThread(NULL,0,http_listener,NULL,0,NULL);

			WaitForSingleObject(listener_init_event,5000);

			CloseHandle(listener_init_event);

			if(m_io_thread == NULL || ! listener_started || listener_error)
				foo_info("error: couldn't create helper thread.");
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
			UnregisterClass((LPCTSTR)g_class_atom, core_api::get_my_instance());
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
		foo_info("error: bind failed.");
	else
	{
		listener_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);

		HANDLE handles[2];
		handles[0] = listener_stop_event;					// stop listening
		handles[1] = WSACreateEvent();						// network activity

		if (handles[1] == WSA_INVALID_EVENT)
			foo_info("WSACreateEvent failed.");
		else
		{
			if (WSAEventSelect(l.m_socket, handles[1], FD_ACCEPT/* | FD_CONNECT | FD_READ | FD_CLOSE*/) != 0)
				foo_info("WSAEventSelect failed. Make sure IP address specified in \"Listen on\" component preferences field is valid and Port:IP combination is not used by another process.");
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
							foo_info("error: WSAWaitForMultipleEvents got invalid handle, stopping listener thread.");

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
								foo_info("error: cannot spawn request processing thread.");
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
		return (* cmd_it->second)(cmd);
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

				commands_map::iterator cmd_it;
				cmd_it = commands.find(command);

				if (cmd_it != commands.end())
				{
//					bool result = (* cmd_it->second)(cmd);
//						result = (* cmd_it->second)(cmd);

					bool result = execute_command(cmd_it, cmd);

					if (!result && cfg.main.log_access)
						foo_info(string_formatter() << "command " << command << " failed.");
				}
				else
					if (cfg.main.log_access && command.get_length())
						foo_info(string_formatter() << "unknown command: " << command << ".");

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
				|| httpc::active_playlist == httpc::pb_playlist	&& httpc::playlist_page_switched)
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

bool cmd_stop(foo_httpserver_command *cmd)
{
	httpc::pb_time = 0;
	static_api_ptr_t<playback_control>()->stop();

	return true;
}

bool cmd_playorpause(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playback_control> pbc;

	pbc->play_or_pause();

	return true;
}

bool cmd_start(foo_httpserver_command *cmd)
{
	pfc::string8 &param1 = cmd->get_param(0);

	if (param1.length())
	{
		static_api_ptr_t<playlist_manager>()->activeplaylist_execute_default_action(atoi(param1));

		httpc::last_action = httpc::FLC_START;
		httpc::should_update_queue = true;
		should_focus_on_playing = true;
	}
	else
	{
		static_api_ptr_t<playback_control> pbc;

		if (pbc->is_playing() && !pbc->is_paused() && pbc->playback_can_seek())
			pbc->playback_seek(0);
		else
		if (pbc->is_paused() || !pbc->is_paused()&&!pbc->is_playing())
			pbc->play_or_pause();
	}

	return true;
}

bool cmd_setfocus(foo_httpserver_command *cmd)
{
	pfc::string8 &param1 = cmd->get_param(0);

	static_api_ptr_t<playlist_manager>()->activeplaylist_set_focus_item(atoi(param1));
	httpc::playlist_item_focused = static_api_ptr_t<playlist_manager>()->activeplaylist_get_focus_item();

	httpc::state_changed |= FSC_PLAYLIST;

	httpc::last_action = httpc::FLC_FOCUS;

	return true;
}

bool cmd_startrandom(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playback_control>()->start(playback_control::track_command_rand, false);

	return true;
}

bool cmd_startprevious(foo_httpserver_command *cmd)
{
	static_api_ptr_t<play_control> pc;

	httpc::pb_time = 0;

	pc->start(playback_control::track_command_prev, false);

	return true;
}

bool cmd_startnext(foo_httpserver_command *cmd)
{
	static_api_ptr_t<play_control> pc;

	httpc::pb_time = 0;
	httpc::update_previouslyplayed();
	pc->start(playback_control::track_command_next, false);

	return true;
}

bool cmd_volume(foo_httpserver_command *cmd)
{
	pfc::string8 &param1 = cmd->get_param(0);

	httpc::set_volume(atoi(param1));

	return true;
}

bool cmd_volume_delta(foo_httpserver_command *cmd)
{
	pfc::string8 &param1 = cmd->get_param(0);

	int vol = httpc::get_volume() + atoi(param1);

	if (vol < 0)
		vol = 0;

	if (vol > 100)
		vol = 100;

	httpc::set_volume(vol);

	return true;
}

bool cmd_volume_db(foo_httpserver_command *cmd)
{
	pfc::string8 &param1 = cmd->get_param(0);

	httpc::volume = (float)(atoi(param1) / -10.0);

	static_api_ptr_t<play_control>()->set_volume(httpc::volume);

	return true;
}

bool cmd_volume_db_delta(foo_httpserver_command *cmd)
{
	static_api_ptr_t<play_control> pc;

	pfc::string8 &param1 = cmd->get_param(0);

	float delta = (float)(atoi(param1) / 10.0);
	float volume = pc->get_volume();

	if (volume + 100.0 < 0.1)
		volume = -66.5;

	volume += delta;

	if (volume < -100)
		volume = -100;

	if (volume > -100 && volume < -66.5)
		volume = -100;

	pc->set_volume(volume);
	httpc::volume = pc->get_volume();

	return true;
}

bool cmd_volume_mute_toggle(foo_httpserver_command *cmd)
{
	static_api_ptr_t<play_control>()->volume_mute_toggle();

	return true;
}

bool cmd_seek(foo_httpserver_command *cmd)
{
	static_api_ptr_t<play_control> pc;

	if (pc->playback_can_seek())
	{
		pfc::string8 param1 = cmd->get_param(0);

		double new_pos = httpc::pb_length * (atoi(param1) / 100.0);
		httpc::pb_time = new_pos;
		pc->playback_seek(new_pos);
		return true;
	}
	else
		return false;
}

bool cmd_seekdelta(foo_httpserver_command *cmd)
{
	static_api_ptr_t<play_control> pc;

	if (pc->playback_can_seek())
	{
		pfc::string8 &param1 = cmd->get_param(0);

		pc->playback_seek_delta(atoi(param1)*1.0);

		return true;
	}
	else
		return false;
}

bool cmd_emptyplaylist(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);

	t_size playlist = ~0;

	if (param1 == "")
		playlist = plm->get_active_playlist();
	else
		playlist = atoi(param1);

	plm->playlist_undo_backup(playlist);
	plm->playlist_clear(playlist);

	httpc::playlist_page = 1;
	httpc::refresh_playing_info();

	return true;
}

bool cmd_undo(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);

	t_size playlist = ~0;

	if (param1 == "")
		playlist = plm->get_active_playlist();
	else
		playlist = atoi(param1);

	bool result = plm->playlist_undo_restore(playlist);
	httpc::refresh_playing_info();

	return result;
}

bool cmd_redo(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);

	t_size playlist = ~0;

	if (param1 == "")
		playlist = plm->get_active_playlist();
	else
		playlist = atoi(param1);

	bool result = plm->playlist_redo_restore(playlist);
	httpc::refresh_playing_info();

	return result;
}

bool cmd_playbackorder(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);

	if (static_cast<t_size>(atoi(param1)) < plm->playback_order_get_count())
	{
		plm->playback_order_set_active(atoi(param1));
		return true;
	}
	else
		return false;
}

bool cmd_playlistitemsperpage(foo_httpserver_command *cmd)
{
	pfc::string8 &param1 = cmd->get_param(0);
	t_size n = atoi(param1);

	if (n > PLAYLIST_ITEMS_PER_PAGE_MAX)
		n = PLAYLIST_ITEMS_PER_PAGE_MAX;

	tcfg.get().playlist_items_per_page = n;
	should_update_playlist = true;
	httpc::should_focus_on_playing = true;
	return true;
}

bool cmd_del(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 param1 = cmd->get_param(0);
	pfc::string8 param2 = cmd->get_param(1);

	t_size playlist = ~0;

	if (param2 == "")
		playlist = plm->get_active_playlist();
	else
		playlist = atoi(param2);

	plm->playlist_undo_backup(playlist);

	bit_array_bittable items(0);
	str_to_bitarray(param1, items);

	plm->playlist_remove_items(playlist, items);

	httpc::last_action = httpc::FLC_REMOVE;

	return true;
}

bool cmd_move(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);
	pfc::string8 &param2 = cmd->get_param(1);

	bit_array_bittable items(0);
	str_to_bitarray(param1, items);

	plm->activeplaylist_undo_backup();

	plm->activeplaylist_set_selection(bit_array_true(), bit_array_false());
	plm->activeplaylist_set_selection(items, bit_array_true());
	plm->activeplaylist_move_selection(atoi(param2));

	httpc::last_action = httpc::FLC_SHIFT; // todo: wtf is this?
	httpc::refresh_playing_info();

	return true;
}

bool cmd_switchplaylist(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);
	pfc::string8 &param2 = cmd->get_param(1);

	plm->set_active_playlist(atoi(param1));
	httpc::playlist_page = 1;
	httpc::should_focus_on_playing = true;

	return true;
}

bool walk_contextmenu(contextmenu_node *node, pfc::string8 &command, pfc::string8 path)
{
	if (node && pfc::strlen_max(node->get_name(), ~0) != 0)
	{
		if (path.get_length())
			path << "/";

		path << node->get_name();

		if (pfc::stricmp_ascii(path, command) == 0)
		{
			if (cfg.main.log_access)
				foo_info(pfc::string_formatter() << "executing " << path);

			node->execute();

			return true;
		}
		else
		if (pfc::string_find_first(command, path, 0) == 0)
		{
			t_size l = node->get_num_children();

			for (t_size i = 0; i < l; ++i)
				if (walk_contextmenu(node->get_child(i), command, path))
					return true;
		}
	}

	return false;
}

bool cmd_playingcommand(foo_httpserver_command *cmd)
{
	pfc::string8 param1 = cmd->get_param(0);

	if (param1.get_length())
	{
		static_api_ptr_t<playlist_manager> plm;
		static_api_ptr_t<contextmenu_manager> cmm;

		if (cmm->init_context_now_playing(contextmenu_manager::flag_view_full))
		{
			contextmenu_node *node = cmm->get_root();
	
			param1 = pfc::string_formatter() << "Root/" << param1;

			plm->playlist_undo_backup(plm->get_playing_playlist());

			if (walk_contextmenu(node, param1, ""))
			{
				httpc::should_update_playlist = true;

				return true;
			}
		}
	}

	return false;
}

bool cmd_selectioncommand(foo_httpserver_command *cmd)
{
	pfc::string8 param1 = cmd->get_param(0);
	pfc::string8 param2 = cmd->get_param(1);

	if (param1.get_length())
	{
		static_api_ptr_t<playlist_manager> plm;
		static_api_ptr_t<contextmenu_manager> cmm;

		if (param2.get_length())
		{
			bit_array_bittable items(0);
			str_to_bitarray(param2, items);

			plm->activeplaylist_set_selection(bit_array_true(), bit_array_false());
			plm->activeplaylist_set_selection(items, bit_array_true());
		}

		pfc::list_t<metadb_handle_ptr> items;
		plm->activeplaylist_get_selected_items(items);

		if (items.get_count())
		{
			cmm->init_context(items, contextmenu_manager::flag_view_full);

			contextmenu_node *node = cmm->get_root();
	
			param1 = pfc::string_formatter() << "Root/" << param1;

			plm->activeplaylist_undo_backup();

			if (walk_contextmenu(node, param1, ""))
			{
				httpc::should_update_playlist = true;

				return true;
			}
		}
	}

	return false;
}

bool cmd_setselection(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);
	pfc::string8 &param2 = cmd->get_param(1);

	t_size playlist = plm->get_active_playlist();

	if (param2.get_length())
		playlist = atoi(param2);

	bit_array_bittable items(0);
	str_to_bitarray(param1, items);

	if (param1.get_length() == 0) // removing all selection
		plm->playlist_set_selection(playlist, bit_array_true(), bit_array_false());
	else
	if (param1.get_length() == 1 && param1.find_first('~') == 0) // selecting everything
		plm->playlist_set_selection(playlist, bit_array_true(), bit_array_true());
	else // selecting specified items
		plm->playlist_set_selection(playlist, items, bit_array_true());

	return true;
}

bool cmd_queueitems(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);

	pfc::list_t<t_size> list;
	str_to_list(param1, list);
	httpc::enqueue(list);
	httpc::last_action = httpc::FLC_ENQUEUE;

	return true;
}

bool cmd_dequeueitems(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);

	pfc::list_t<t_size> list;
	str_to_list(param1, list);
	httpc::dequeue(list);
	httpc::last_action = httpc::FLC_DEQUEUE;

	return true;
}

bool cmd_saq(foo_httpserver_command *cmd)
{
	if (cfg.main.stop_after_queue_enable)
	{
		pfc::string8 &param1 = cmd->get_param(0);

		cfg.misc.stop_after_queue = atoi(param1) == 1 ? true : false;
	}

	return true;
}

bool cmd_sac(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playback_control> pc;
	pfc::string8 &param1 = cmd->get_param(0);

	if (atoi(param1) == 1)
		pc->set_stop_after_current(true);
	else
		pc->set_stop_after_current(false);

	httpc::sac = pc->get_stop_after_current();

	return true;
}

bool cmd_flushqueue(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;

	plm->queue_flush();

	return true;
}

bool cmd_volumeup(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playback_control> pc;
	pfc::string8 &param1 = cmd->get_param(0);

	byte volume = httpc::get_volume();
	byte diff = atoi(param1);
	byte new_volume = volume + diff > 100? 100 : volume + diff;
	httpc::set_volume(new_volume);

	return true;
}

bool cmd_volumedown(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playback_control> pc;
	pfc::string8 &param1 = cmd->get_param(0);

	byte volume = httpc::get_volume();
	byte diff = atoi(param1);
	byte new_volume = volume - diff < 0? 0 : volume - diff;
	httpc::set_volume(new_volume);

	return true;
}

bool cmd_queuealbum(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playback_control> pc;
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);

	t_size focus;

	if (param1.get_length() == 0)
		focus = plm->activeplaylist_get_focus_item();
	else
		focus = atoi(param1);

	if (focus != ~0 
		&& focus < plm->activeplaylist_get_item_count()
		&& plm->activeplaylist_get_item_count() > 0 
		&& plm->queue_get_count() < 64)
	{
		service_ptr_t<titleformat_object> album_script;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(album_script,"%ARTIST% - %ALBUM% - %DATE%");
		
		pfc::string8 first, next;

		t_size count = 0;

		if (plm->activeplaylist_get_item_handle(focus)->format_title(NULL, first, album_script, NULL))
		{
			t_size len = plm->activeplaylist_get_item_count();
			for (t_size idx = focus; idx < len && plm->queue_get_count() < 64; ++idx)
			{
				next.reset();

				if (plm->activeplaylist_get_item_handle(idx)->format_title(NULL, next, album_script, NULL) && next == first)
				{
					plm->queue_add_item_playlist(plm->get_active_playlist(), idx);
					++count;
				}
				else
					break;
			}
		}
		album_script.release();

		return true;
	}
	else
		return false;
}

// todo: improve efficiency
bool cmd_queuerandomitems(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playback_control> pc;
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 &param1 = cmd->get_param(0);

	t_size items_to_be_queued = atoi(param1);
	t_size items_queued_in_active_playlist = 0;
	t_size items_available_in_active_playlist = 0;
	t_size active_playlist = plm->get_active_playlist();

	pfc::list_t<metadb_handle_ptr> playlist_items;
	pfc::list_t<t_playback_queue_item> queue;
	plm->queue_get_contents(queue);

	bit_array_bittable mask(plm->activeplaylist_get_item_count());

	t_size l = queue.get_count();
	for (t_size k = 0; k < l; ++k)
		if (queue[k].m_playlist == active_playlist
			&& !mask.get(queue[k].m_item))
		{
			mask.set(queue[k].m_item, true);
			++items_queued_in_active_playlist;
		}


	plm->activeplaylist_get_all_items(playlist_items);

	playlist_items.remove_mask(mask);

	items_available_in_active_playlist = plm->activeplaylist_get_item_count() - items_queued_in_active_playlist;

	if (items_to_be_queued == 0)
		items_to_be_queued = 1;

	if (items_to_be_queued > items_available_in_active_playlist)
		items_to_be_queued = items_available_in_active_playlist;
						
	if (items_to_be_queued > 64)
		items_to_be_queued = 64;

	srand((unsigned int)__rdtsc());
	for (;items_to_be_queued > 0; --items_to_be_queued)
	{
		t_size p_item_random;
		t_size p_playlist_item;

		p_item_random = rand() % (playlist_items.get_count());

		if (plm->playlist_find_item(active_playlist, playlist_items[p_item_random], p_playlist_item))
		{
			plm->queue_add_item_playlist(active_playlist, p_playlist_item);
			playlist_items.remove_by_idx(p_item_random);
		}
	}

	return true;
}

bool cmd_queryadvance(foo_httpserver_command *cmd)
{
	pfc::string8 &param1 = cmd->get_param(0);

	if (param1.get_length())
	{
		httpc::facets::next_facet(param1);
		httpc::facets::fill_playlist();
		httpc::should_update_playlist = true;
		httpc::playlist_page = 1;
		return true;
	}
	else
		return false;
}

bool cmd_queryretrace(foo_httpserver_command *cmd)
{
	httpc::facets::prev_facet();
	httpc::facets::fill_playlist();
	httpc::should_update_playlist = true;
	httpc::should_update_playlists_list = true;

	return true;
}

bool cmd_searchmedialibrary(foo_httpserver_command *cmd)
{
	pfc::string8 &param1 = cmd->get_param(0);

	static_api_ptr_t<autoplaylist_manager> apm;
	static_api_ptr_t<playlist_manager> plm;

	t_size playlist;
	
	if (cfg.query.sendtodedicated)
		playlist = plm->find_or_create_playlist(cfg.misc.query_playlist_name, pfc::infinite_size);
	else
		playlist = plm->get_active_playlist();
	
	if (playlist != pfc::infinite_size)
	{
		plm->set_active_playlist(playlist);
		
		if (apm->is_client_present(playlist))
			apm->remove_client(playlist);
	
		httpc::autoplaylist_request = param1;

		if (param1.get_length())
		{
			plm->playlist_undo_backup(playlist);

			try
			{
				apm->add_client_simple(param1.toString(), cfg.query.sortpattern, playlist, autoplaylist_flag_sort);
			}
			catch (pfc::exception &e)
			{
				console::error(e.what());
			}

			if (apm->is_client_present(playlist))
				apm->remove_client(playlist);
		}

		plm->set_active_playlist(playlist);
		httpc::should_update_playlist = true;
		httpc::query_playlist = playlist;
		httpc::should_update_playlists_list = true;
		httpc::playlist_page = 1;
		return true;

	}
	else
		return false;
}


bool cmd_cmdline(foo_httpserver_command *cmd)
{
/*	if (cfg.main.allow_commandline)
	{*/
		pfc::string8 &param1 = cmd->get_param(0);

		pfc::stringcvt::string_wide_from_utf8 cmd_w (httpc::fb2k_path);
		pfc::stringcvt::string_wide_from_utf8 par_w (param1);

		if (cfg.main.log_access)
			foo_info(pfc::string_formatter() << "Executing " << httpc::fb2k_path << " " << param1);

		if ((int)ShellExecute(0, L"open", cmd_w, par_w, 0, SW_NORMAL) <= 32)
			return false;

		httpc::should_update_playlist = true;
		httpc::should_update_playlists_list = true;
		httpc::should_update_queue = true;
		httpc::should_update_playlist_total_time = true;

		return true;
/*	}
	else
		return false;*/
}

bool cmd_switchplaylistpage(foo_httpserver_command *cmd)
{
	if (tcfg.get().playlist_items_per_page == 0)
		return false;

	pfc::string8 &param1 = cmd->get_param(0);
	static_api_ptr_t<playlist_manager> plm;
	t_size page = atoi(param1);

	t_size total_pages = (t_size)ceil(plm->activeplaylist_get_item_count()*1.0 / tcfg.get().playlist_items_per_page);

	if (page > total_pages)
		page = total_pages;

	if (page == 0)
		page = 1;

	httpc::playlist_page = page;
	httpc::should_update_playlist = true;
	httpc::should_update_queue = true;

	return true;
}

bool cmd_browse(foo_httpserver_command *cmd)
{
	if (httpc::enqueueing)
		return false;

	pfc::string8 &param1 = cmd->get_param(0);
	pfc::string8 &param2 = cmd->get_param(1);

	foo_browsefiles browser;
	list_t<const char *> files;	// files/dirs to be enqueued
	pfc::string8 filename = param1;

	if (httpc::is_extension_registered(filename) != pfc::infinite_size && param2.get_length() == 0 // adding a single file 
		|| strcmp(param2, "EnqueueDirSubdirs") == 0) // adding a nested directory
		files.add_item(filename);
	else
	if (strcmp(param2, "EnqueueDir") == 0) // adding a directory without nesting;
	{
		browser.browse(const_cast<char *>(filename.operator const char *()));

		t_size l = browser.entries.get_count();
		for(unsigned int i = 0; i < l; ++i)
			if (browser.entries[i].type != foo_browsefiles::ET_DIR)
					files.add_item(browser.entries[i].path);
	}

	if (files.get_count())	// if there is anything to enqueue
	{
		static_api_ptr_t<playlist_manager>()->activeplaylist_undo_backup();

		service_ptr_t<process_locations_notify_my> notify = new service_impl_t<process_locations_notify_my>();
		httpc::enqueueing = true;

		static_api_ptr_t<playlist_incoming_item_filter_v2>()->process_locations_async(
			files,
			playlist_incoming_item_filter_v2::op_flag_background | playlist_incoming_item_filter_v2::op_flag_delay_ui,
			httpc::restrict_mask,
			"",
			core_api::get_main_window(),
			notify
		);

	}

	return true;
}

bool cmd_parse(foo_httpserver_command *cmd)
{
	return true;
}

bool cmd_removeplaylist(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;

	if (cmd->get_param(0))
	{
		t_size playlist = atoi(cmd->get_param(0));
		return plm->remove_playlist_switch(playlist);
	}
	else
		return plm->remove_playlist_switch(plm->get_active_playlist());
}

bool cmd_createplaylist(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 param1 = cmd->get_param(0);
	pfc::string8 param2 = cmd->get_param(1);

	t_size pindex;

	if (!param2.get_length() && plm->get_active_playlist() != pfc::infinite_size)
		pindex = plm->get_active_playlist() + 1;
	else
	if (param2.get_length())
		pindex = atoi(param2) + 1;
	else	
		pindex = pfc::infinite_size;

	if (param1.get_length())
		plm->create_playlist(param1, pfc::infinite_size, pindex);
	else
		plm->create_playlist_autoname(pindex);

	return true;
}


bool cmd_renameplaylist(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 param1 = cmd->get_param(0);
	pfc::string8 param2 = cmd->get_param(1);

	t_size pindex;

	if (param2.get_length())
		pindex = atoi(param2);
	else
		pindex = plm->get_active_playlist();

	if (param1.get_length())
		plm->playlist_rename(pindex, param1, pfc::infinite_size);

	return true;
}

bool cmd_refreshplayinginfo(foo_httpserver_command *cmd)
{
	httpc::refresh_playing_info();

	return true;
}

bool cmd_sort(foo_httpserver_command *cmd)
{
	bool sort_selection_only;
	static_api_ptr_t<playlist_manager> plm;
	pfc::string8 param1 = cmd->get_param(0);
	pfc::string8 param2 = cmd->get_param(1);

	if (param1.get_length() == 0) // no sort pattern specified, ABORT ABORT ABORT
		return false;

	plm->activeplaylist_undo_backup();

	if (param2.get_length()) 
	{
		// sorting selection
		bit_array_bittable items(0);
		str_to_bitarray(param2, items);

		plm->activeplaylist_set_selection(bit_array_true(), bit_array_false());
		plm->activeplaylist_set_selection(items, bit_array_true());

		sort_selection_only = true;
	}
	else
	{
		// sorting whole activeplaylist
		sort_selection_only = false;
	}

	return plm->activeplaylist_sort_by_format(param1.toString(), sort_selection_only);
}

bool cmd_focusonplaying(foo_httpserver_command *cmd)
{
	static_api_ptr_t<playlist_manager> plm;
	static_api_ptr_t<playback_control> pc;

	t_size playing_playlist = plm->get_playing_playlist();
	t_size active_playlist = plm->get_active_playlist();
	bool is_playing = pc->is_playing() || pc->is_paused();

	if (playing_playlist != pfc::infinite_size && playing_playlist != active_playlist && is_playing)
		plm->set_active_playlist(playing_playlist);

	if (is_playing)
	{
		httpc::should_focus_on_playing = true;
		httpc::should_update_playlist = true;	
	}

	return true;
}

void gen_cmd_table()
{
	commands["P"] = &cmd_switchplaylistpage;
	commands["SwitchPlaylistPage"] = &cmd_switchplaylistpage;
	commands["Stop"] = &cmd_stop;
	commands["Start"] = &cmd_start;
	commands["PlayOrPause"] = &cmd_playorpause;
	commands["StartPrevious"] = &cmd_startprevious;
	commands["StartNext"] = &cmd_startnext;
	commands["Seek"] = &cmd_seek;
	commands["SeekDelta"] = &cmd_seekdelta;
	commands["Browse"] = &cmd_browse;
	commands["Search"] = &cmd_searchmedialibrary;
	commands["SearchMediaLibrary"] = &cmd_searchmedialibrary;
	commands["SetFocus"] = &cmd_setfocus; 
	commands["StartRandom"] = &cmd_startrandom;
	commands["Volume"] = &cmd_volume;
	commands["VolumeDelta"] = &cmd_volume_delta;
	commands["VolumeDB"] = &cmd_volume_db;
	commands["VolumeDBDelta"] = &cmd_volume_db_delta;
	commands["VolumeMuteToggle"] = &cmd_volume_mute_toggle;
	commands["Parse"] = &cmd_parse;
	commands["RefreshPlayingInfo"] = &cmd_refreshplayinginfo;
	commands["Del"] = &cmd_del;
	commands["Move"] = &cmd_move;
	commands["SwitchPlaylist"] = &cmd_switchplaylist;
	commands["PlayingCommand"] = &cmd_playingcommand;
	commands["SelectionCommand"] = &cmd_selectioncommand;
	commands["EnqueueTrack"] = &cmd_queueitems;
	commands["QueueItems"] = &cmd_queueitems;
	commands["DequeueItems"] = &cmd_dequeueitems;
	commands["DequeueTrack"] = &cmd_dequeueitems;
	commands["SAQ"] = &cmd_saq;
	commands["SAC"] = &cmd_sac;
	commands["FlushQueue"] = &cmd_flushqueue;
	commands["QueueAlbum"] = &cmd_queuealbum;
	commands["QueueRandomItems"] = &cmd_queuerandomitems;
	commands["QueryRetrace"] = &cmd_queryretrace;
	commands["QueryAdvance"] = &cmd_queryadvance;
	commands["CmdLine"] = &cmd_cmdline;
	commands["RemovePlaylist"] = &cmd_removeplaylist;
	commands["CreatePlaylist"] = &cmd_createplaylist;
	commands["RenamePlaylist"] = &cmd_renameplaylist;
	commands["FocusOnPlaying"] = &cmd_focusonplaying;
	commands["EmptyPlaylist"] = &cmd_emptyplaylist;
	commands["Undo"] = &cmd_undo;
	commands["Redo"] = &cmd_redo;
	commands["PlaybackOrder"] = &cmd_playbackorder;
	commands["PlaylistItemsPerPage"] = &cmd_playlistitemsperpage;
	commands["SetSelection"] = &cmd_setselection;
	commands["Sort"] = &cmd_sort;
	commands["VolumeUp"] = &cmd_volumeup; // deprecated
	commands["VolumeDown"] = &cmd_volumedown; // deprecated
}

}
}
