#include "stdafx.h"
#include "httpcontrol.h"

struct sac {
	class callback_sac_off : public main_thread_callback
	{
	public:
		virtual void callback_run()
		{
			static_api_ptr_t<playback_control>()->set_stop_after_current(false);
			httpc::sac = false;
		}
	};

	class callback_sac_on : public main_thread_callback
	{
	public:
		virtual void callback_run()
		{
			static_api_ptr_t<playback_control>()->set_stop_after_current(true);
			httpc::sac = true;
		}

	};

	static void on () {
			service_ptr_t<callback_sac_on> myc = new service_impl_t<callback_sac_on>();
			static_api_ptr_t<main_thread_callback_manager>()->add_callback(myc);
	}

	static void off() {
			service_ptr_t<callback_sac_off> myc = new service_impl_t<callback_sac_off>();
			static_api_ptr_t<main_thread_callback_manager>()->add_callback(myc);
	}
};

class callbacks : private play_callback_impl_base, private playlist_callback_impl_base {
public:
	void on_playback_starting(play_control::t_track_command p_command, bool p_paused)
	{
		httpc::state_changed |= httpc::FSC_PLAYBACK;
		httpc::pb_state = p_paused ? httpc::FOO_PAUSED : httpc::FOO_PLAYING;

		httpc::should_focus_on_playing = true;
	};

	void on_playback_new_track(metadb_handle_ptr p_track)
	{
		httpc::state_changed |= httpc::FSC_PLAYBACK;

		if (cfg.misc.stop_after_queue && static_api_ptr_t<playlist_manager>()->queue_get_count() == 1)
		{
			httpc::reset_sac = true;
			sac::on();
		}

		httpc::should_focus_on_playing = true;
		httpc::update_previouslyplayed();
		httpc::pb_item = pfc::infinite_size ;
		httpc::pb_time = 0;
	};

	void on_playback_stop(play_control::t_stop_reason reason)
	{
		httpc::state_changed |= httpc::FSC_PLAYBACK;

		httpc::update_previouslyplayed();
		httpc::empty_info();

		if (cfg.misc.stop_after_queue && httpc::reset_sac)
		{
			httpc::reset_sac = false;
			sac::off();
		}
	};

	void on_playback_seek(double p_time)
	{
		httpc::state_changed |= httpc::FSC_PLAYBACK;
		httpc::pb_time = p_time;
	};

	void on_playback_pause(bool p_state)
	{
		httpc::state_changed |= httpc::FSC_PLAYBACK;
		httpc::pb_state = p_state ? httpc::FOO_PAUSED : httpc::FOO_PLAYING;
	};

	void on_playback_time(double p_time)
	{
		httpc::pb_time = p_time;
	};

	void on_volume_change(float p_new_val)
	{
		httpc::state_changed |= httpc::FSC_VOLUME;
		httpc::refresh_volume(p_new_val);
	};

	void on_items_added(t_size p_playlist,t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const pfc::bit_array & p_selection)
	{
		if (static_api_ptr_t<playlist_manager>()->get_active_playlist() == p_playlist)
		{
			httpc::should_update_playlist = true;	
			httpc::should_update_playlist_total_time = true;
			httpc::should_update_queue = true;
			httpc::state_changed |= httpc::FSC_PLAYLIST;
		}

		httpc::should_update_playlists_list = true;
	};

	void on_items_reordered(t_size p_playlist,const t_size * p_order,t_size p_count)
	{
		if (p_order && p_count && (p_playlist == static_api_ptr_t<playlist_manager>()->get_active_playlist()))
		{
			httpc::should_update_playlist = true;	
			httpc::state_changed |= httpc::FSC_PLAYLIST;
			httpc::should_update_queue = true;
			httpc::should_focus_on_playing = true;
		}
	};

	void on_items_removed(t_size p_playlist,const pfc::bit_array & p_mask,t_size p_old_count,t_size p_new_count)
	{
		if (p_playlist == static_api_ptr_t<playlist_manager>()->get_active_playlist())
		{
			httpc::should_update_playlist = true;	
			httpc::should_update_playlist_total_time = true;
			httpc::should_update_queue = true;
			httpc::state_changed |= httpc::FSC_PLAYLIST;
		}
		httpc::should_update_playlists_list = true;
		httpc::empty_previouslyplayed();
	};

	void on_item_focus_change(t_size p_playlist,t_size p_from,t_size p_to)
	{
		httpc::state_changed |= httpc::FSC_PLAYLIST;
	};

	void on_items_modified(t_size p_playlist,const pfc::bit_array & p_mask)
	{
		if (p_playlist == static_api_ptr_t<playlist_manager>()->get_active_playlist())
		{
			httpc::should_update_playlist = true;
			httpc::state_changed |= httpc::FSC_PLAYLIST;
			httpc::should_update_queue = true;
		}
	};

	void on_items_replaced(t_size p_playlist,const pfc::bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data)
	{
		if (p_playlist == static_api_ptr_t<playlist_manager>()->get_active_playlist())
		{
			httpc::should_update_playlist = true;
			httpc::should_update_playlist_total_time = true;
			httpc::state_changed |= httpc::FSC_PLAYLIST;
			httpc::should_update_queue = true;
			httpc::should_focus_on_playing = true;
		}
	};

	void on_playlist_activate(t_size p_old,t_size p_new)
	{
		httpc::should_update_playlist = true;	
		httpc::should_update_playlist_total_time = true;
		httpc::should_update_queue = true;
		httpc::state_changed |= httpc::FSC_PLAYLIST;
		httpc::should_focus_on_playing = true;
	};

	void on_playlist_created(t_size p_index,const char * p_name,t_size p_name_len)
	{
		if (p_index == static_api_ptr_t<playlist_manager>()->get_active_playlist())
		{
			httpc::should_update_playlist = true;	
			httpc::should_update_playlist_total_time = true;
			httpc::state_changed |= httpc::FSC_PLAYLIST | httpc::FSC_PLAYLISTS;
		}
		httpc::should_update_playlists_list = true;
	};

	void on_playlists_reorder(const t_size * p_order,t_size p_count)
	{
		httpc::query_playlist = static_api_ptr_t<playlist_manager>()->find_playlist(cfg.misc.query_playlist_name);
		httpc::should_update_playlists_list = true;
		httpc::state_changed |= httpc::FSC_PLAYLISTS;
	};

	void on_playlists_removed(const pfc::bit_array & p_mask,t_size p_old_count,t_size p_new_count)
	{
		httpc::query_playlist = static_api_ptr_t<playlist_manager>()->find_playlist(cfg.misc.query_playlist_name);
		httpc::should_update_playlist = true;	
		httpc::should_update_playlists_list = true;
		httpc::state_changed |= httpc::FSC_PLAYLISTS;
	};

	void on_playlist_renamed(t_size p_index,const char * p_new_name,t_size p_new_name_len)
	{
		if (p_index == httpc::query_playlist)
			cfg.misc.query_playlist_name = p_new_name;
		httpc::state_changed |= httpc::FSC_PLAYLIST;
		httpc::should_update_playlists_list = true;
	};

	void on_playback_order_changed(t_size p_new_index)
	{
		httpc::state_changed |= httpc::FSC_PLAYBACK_ORDER;	
	};
};

class oninitquit : public initquit {
private:
	callbacks *callbacks_ptr;

public:
    oninitquit() { callbacks_ptr = NULL; }

	virtual void on_init()
	{
		httpc::empty_info();
		httpc::empty_previouslyplayed();
		httpc::control::set_active(cfg.main.startserver);
		httpc::ui::gen::gen_func_table();
		httpc::control::gen_cmd_table();

		InitializeCriticalSection(&httpc::cs);

		// get registered extensions list
		httpc::get_registered_extensions();
		httpc::control_credentials_auth_hash_update();

		// getting hang of where do we get templates and stuff
		httpc::fb2k_profile_path = pfc::string8(file_path_display(core_api::get_profile_path())) << "\\foo_httpcontrol_data";
		httpc::choose_srv_home_dir();

		// getting fb2k executable path
		HMODULE instance = GetModuleHandleW(NULL);
		wchar_t path[MAX_PATH];
		if (GetModuleFileNameW(instance, path, MAX_PATH) != 0)
			httpc::fb2k_path = pfc::stringcvt::string_utf8_from_wide((const wchar_t*)path);
		else
			foo_info("coudln't retrieve foobar2000 executable path");

		httpc::build_restrict_to_path_list();

		// find out Query playlist index for later use
		static_api_ptr_t<playlist_manager> plm;
		httpc::query_playlist = plm->find_playlist(cfg.misc.query_playlist_name);

		// filling in initial data
		httpc::should_update_playlist = true;	
		httpc::should_update_queue = true;
		httpc::should_update_playlists_list = true;
		httpc::should_focus_on_playing = true;

		callbacks_ptr = new callbacks;

		service_enum_t<contextmenu_item> e;
		service_ptr_t<contextmenu_item> ptr;
	};

	virtual void on_quit()
	{
		if (httpc::control::is_active())
			httpc::control::set_active(false);

		httpc::script_playlist_row.release();
		httpc::script_helper1.release();
		httpc::script_helper2.release();
		httpc::script_helper3.release();

		delete callbacks_ptr;

		DeleteCriticalSection(&httpc::cs);
	};
};

static initquit_factory_t<oninitquit> g_oninitquit;

class queue_foo_httpcontrol : public playback_queue_callback
{
public:
	void on_changed(t_change_origin p_origin)
	{
		if (cfg.misc.stop_after_queue)
		{
			if (static_api_ptr_t<playback_control>()->is_playing())
			{
				if (static_api_ptr_t<playlist_manager>()->queue_get_count() == 0)
				{
					httpc::reset_sac = true;
					sac::on();
				}
				else
				{
					httpc::reset_sac = false;
					sac::off();
				}
			}
		}
		httpc::should_update_queue = true;
		httpc::should_update_playlist = true;	
	}
};

static play_callback_static_factory_t<queue_foo_httpcontrol> foo_queue_playback;
