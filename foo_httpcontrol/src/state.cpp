#include "stdafx.h"

#include "state.h"
#include "config.h"
#include "browsefiles.h"
#define _CRT_RAND_S
#include <stdlib.h>

namespace httpc {
	service_ptr_t<titleformat_object> script_playlist_row;
	service_ptr_t<titleformat_object> script_helper1;
	service_ptr_t<titleformat_object> script_helper2;
	service_ptr_t<titleformat_object> script_helper3;
	service_ptr_t<titleformat_object> script_albumart_lookup;

	CRITICAL_SECTION cs;

	bool	enqueueing;	// if enqueueing is in progress;

	size_t		pb_state;				// current foobar state
	size_t		last_action;			// previously selected playlist action
	size_t		state_changed;			// previous state change
	float		volume;
	pfc::string8 pb_helper1;
	pfc::string8 pb_helper2;
	pfc::string8 pb_helper3;
	pfc::string8 pb_helper1x;
	pfc::string8 pb_helper2x;
	pfc::string8 pb_helper3x;
	pfc::string8 pb_albumart_path;
	pfc::string8 pb_albumart_path_crc32;
	pfc::string8 pb_albumart_lookup;
	pfc::string8 pb_albumart_embedded_file;
	albumart_status pb_albumart_status;
	album_art_data_ptr pb_albumart_embedded_ptr;
	size_t		pb_item;				// playing item
	double		pb_length;				// playing item length
	double		pb_time;				// playing item position
	size_t		pb_playlist;			// playing playlist
	size_t		pb_playlist_item_count;	// playing playlist item count
	size_t		pb_item_prev;			// previously played item
	size_t		pb_playlist_prev;		// previously played playlist
	size_t		playlist_item_focused;	// focused item of current playlist
	size_t		playlist_page;			// page number of current playlist
	size_t		active_playlist;
	size_t		active_playlist_item_count;
	size_t		query_playlist;
	bool			playlist_page_switched;
	bool			should_focus_on_playing;
	bool			should_update_playlist;
	bool			should_update_playlist_total_time;
	bool			should_update_playlists_list;
	bool			should_update_queue;
	pfc::list_t<playlist_entry,pfc::alloc_fast_aggressive> playlist_view;
	pfc::string_simple	playlist_total_time;
	pfc::string_simple	queue_total_time;
	bool			sac;							// stop after current flag
	bool			reset_sac;						// reset stop after current after track stop flag;
	bool			active_playlist_is_undo_available;
	bool			active_playlist_is_redo_available;
	pfc::string_simple control_credentials_auth_hash;
	pfc::list_t<pfc::string_simple> extensions;		// registered extensions
	pfc::list_t<pfc::string_simple> extension_names;// registered extension names
	pfc::string8	restrict_mask;					// restrict mask based on registered extensions
	pfc::list_t<playlist_info> playlist_list;		// list of playlists
	pfc::string8 fb2k_profile_path;
	pfc::string8 fb2k_path;
	pfc::string8 srv_home_dir;
	pfc::string_simple autoplaylist_request;

	pfc::hires_timer timer;

	class albumart_apev2_extractor : public album_art_extractor_impl_stdtags {
		albumart_apev2_extractor();
	};


	void update_previouslyplayed()
	{
		if (pb_item != pfc::infinite_size  && pb_playlist != pfc::infinite_size)
		{
			pb_item_prev = pb_item;
			pb_playlist_prev = pb_playlist;
		}
	}

	void enqueue(pfc::list_t<t_size> &list)
	{
		static_api_ptr_t<playlist_manager> plm;

		if (list.get_count())
			plm->activeplaylist_set_focus_item(list[list.get_count()-1]);

		for (t_size i = 0; i < list.get_count(); ++i)
			plm->queue_add_item_playlist(plm->get_active_playlist(), list.get_item(i));

		state_changed |= FSC_PLAYLIST;
	}

	void dequeue(pfc::list_t<t_size> &list)
	{
		static_api_ptr_t<playlist_manager> plm;

		if (list.get_count())
			plm->activeplaylist_set_focus_item(list[list.get_count()-1]);

		for (t_size j = 0; j < list.get_count(); ++j)
		{
			size_t queue_item = pfc::infinite_size;

			pfc::list_t<t_playback_queue_item> queue;
			plm->queue_get_contents(queue);

			if (queue.get_count() && plm->activeplaylist_get_item_count() > list.get_item(j))
			for (unsigned int i = 0; i < queue.get_count() ; ++i)
			{
				t_playback_queue_item &qi = queue[i];

				if (qi.m_item == list.get_item(j)
					&& qi.m_playlist == active_playlist
					&& plm->activeplaylist_get_item_handle(list.get_item(j)) == qi.m_handle)
				{
					queue_item = i;
					break;
				}
			}

			if (queue_item != pfc::infinite_size)
				plm->queue_remove_mask(bit_array_one(queue_item));
		}

		state_changed |= FSC_PLAYLIST;
	}

	void empty_info()
	{
		pb_state = httpc::FOO_STOPPED;
		pb_time = 0;
		pb_length = 0;
		pb_helper1.reset();
		pb_helper2.reset();
		pb_helper3.reset();
		pb_helper1x.reset();
		pb_helper2x.reset();
		pb_helper3x.reset();
		pb_item = pfc::infinite_size;
		pb_playlist = pfc::infinite_size;
		pb_playlist_item_count = pfc::infinite_size;
		pb_albumart_path.reset();
		pb_albumart_path_crc32 = tcfg.get().albumart_not_available;
		pb_albumart_embedded_ptr.release();
		pb_albumart_status = httpc::AS_NO_INFO;
		active_playlist_item_count = pfc::infinite_size;
		state_changed = FSC_NONE;
	}

	void empty_previouslyplayed()
	{
		pb_item_prev = pfc::infinite_size;
		pb_playlist_prev = pfc::infinite_size;
	}

	void refresh_volume(float p_volume)
	{
		static_api_ptr_t<playback_control> plc;

		if ((abs(p_volume) - 1.0) < 0.001)
			httpc::volume = plc->get_volume();
		else
			httpc::volume = p_volume;
	}

	void set_volume(t_size percent)
	{
		static_api_ptr_t<playback_control> pc;

		if (percent > 100)
			percent = 100;

		float scale = (float)(percent / 100.0);
		float gain = (float)(20.0 * log10(scale));
		pc->set_volume(gain);
		httpc::volume = pc->get_volume();
	}

	byte get_volume()
	{
		return (byte)(pow(10.0,httpc::volume / 20.0) * 100.0);
	}

	void retrieve_albumart(metadb_handle_ptr pb_item_ptr)
	{
		pb_albumart_status = AS_NOT_FOUND;

		// looking for an external albumart by predefined path mask pb_albumart_lookup
		if (pb_albumart_lookup.get_length()>0)
		{
			pfc::list_t<pfc::string8> lookups;

			get_list(pb_albumart_lookup, lookups, '|', false);

			HANDLE hFind;
			LPWIN32_FIND_DATAW findFileData;
			findFileData = new WIN32_FIND_DATAW;

			for (size_t i = 0; i < lookups.get_count(); ++i)
			{
				pfc::stringcvt::string_wide_from_utf8 mask(lookups[i]);

				hFind = FindFirstFileW(mask.get_ptr(), findFileData);

				if (hFind != INVALID_HANDLE_VALUE)
				{
					pfc::stringcvt::string_utf8_from_wide filename(findFileData->cFileName);
					pfc::string_directory directory(lookups[i]);
					
					// todo: check for file size
					t_size filesize = findFileData->nFileSizeHigh * ((long)MAXDWORD+1) + findFileData->nFileSizeLow;

					pb_albumart_path = directory;

					if (pb_albumart_path.get_length())
						pb_albumart_path.fix_dir_separator('\\');

					pb_albumart_path << filename;

					pb_albumart_path_crc32 = pfc::string_formatter() << "/" << pfc::string8(tcfg.get().root) << pfc::string8("/albumart_") << calcCRC(pb_albumart_path, strlen(pb_albumart_path)) << "." << pfc::string_extension(filename);

					FindClose(hFind);

					if (tcfg.get().albumart_limit_size != 0 && filesize <= tcfg.get().albumart_limit_size || tcfg.get().albumart_limit_size == 0)
						pb_albumart_status = AS_FILE;
					else
						pb_albumart_status = AS_NOT_FOUND;

					break;
				}
			}
			delete findFileData;
		}
	
		if ( /*cfg.main.albumart_embedded_retrieve */
			pb_albumart_status & AS_NOT_FOUND || tcfg.get().albumart_prefer_embedded )
		{
			abort_callback_dummy p_abort;
			album_art_manager_instance_ptr aami = static_api_ptr_t<album_art_manager>()->instantiate();
			if (aami->open(pb_item_ptr->get_path(), p_abort))
			{
				pb_albumart_embedded_ptr.release();
				pb_albumart_status = AS_MEMORY;
				try
				{
					pb_albumart_embedded_ptr = aami->query(album_art_ids::cover_front, p_abort);
				}
				catch (...)
				{
					try
					{
						pb_albumart_embedded_ptr = aami->query(album_art_ids::cover_back, p_abort);
					}
					catch (...)
					{
						try
						{
							pb_albumart_embedded_ptr = aami->query(album_art_ids::disc, p_abort);
						}
						catch (...)
						{
							try
							{
								pb_albumart_embedded_ptr = aami->query(album_art_ids::icon, p_abort);
							}
							catch (...)
							{
								if (pb_albumart_status != AS_FILE)
									pb_albumart_status = AS_NOT_FOUND;
							}
						}
					}
				}

				if (pb_albumart_status == AS_MEMORY 
					&& pb_albumart_embedded_ptr.is_valid() 
					&& pb_albumart_embedded_ptr->get_size()
					&& ((tcfg.get().albumart_limit_size != 0) && (pb_albumart_embedded_ptr->get_size() <= tcfg.get().albumart_limit_size) || tcfg.get().albumart_limit_size == 0 ))
				{
					pb_albumart_embedded_file = file_path_display(pb_item_ptr->get_path());
					pb_albumart_path_crc32 = pfc::string_formatter() << "/" << pfc::string8(tcfg.get().root) <<pfc::string8("/albumart_") << calcCRC(pb_albumart_embedded_file, pb_albumart_embedded_file.get_length());
				}
				else
				{
					if (pb_albumart_status != AS_FILE)
						pb_albumart_status = AS_NOT_FOUND;
				}
			}
			aami.release();
		}

		if (pb_albumart_status & AS_NOT_FOUND)
			pb_albumart_path_crc32 = tcfg.get().albumart_not_found;
	}

	void refresh_playing_info()
	{
		if (!httpc::control::is_active() || !tcfg.get_count())
			return;

		static_api_ptr_t<playlist_manager> plm;
		static_api_ptr_t<playback_control> plc;
		static_api_ptr_t<play_control> pc;
		metadb_handle_ptr pb_item_ptr;
		t_size index;

		pb_item = pfc::infinite_size;
		pb_playlist = pfc::infinite_size;

		if(pc->get_now_playing(pb_item_ptr))
			if(plm->get_playing_item_location(&pb_playlist, &index))
				pb_item = index;

		if (pb_playlist != pfc::infinite_size)
			pb_playlist_item_count = plm->playlist_get_item_count(pb_playlist);
		else
			pb_playlist_item_count = pfc::infinite_size;
			
		if (plm->get_playlist_count() == 1)
			plm->set_active_playlist(0);

		active_playlist = plm->get_active_playlist();

		playlist_item_focused = plm->playlist_get_focus_item(plm->get_active_playlist());

		active_playlist_item_count = plm->playlist_get_item_count(plm->get_active_playlist());

		t_size page_prev = httpc::playlist_page;

		if (tcfg.get().playlist_items_per_page != 0)
		{
			if (should_focus_on_playing)
			{
				if (active_playlist == pb_playlist && pb_item != pfc::infinite_size)
					playlist_page = (t_size)ceil((pb_item+1)*1.0 / tcfg.get().playlist_items_per_page*1.0);
				else
				if (playlist_item_focused != pfc::infinite_size)
					playlist_page = (t_size)ceil((playlist_item_focused+1)*1.0 / tcfg.get().playlist_items_per_page*1.0);

				should_focus_on_playing = false;
			}

			if (playlist_page == pfc::infinite_size || playlist_page == 0)
				playlist_page = 1;

			if (playlist_page > (t_size)ceil(active_playlist_item_count*1.0 / tcfg.get().playlist_items_per_page*1.0))
				playlist_page = (t_size)ceil(active_playlist_item_count*1.0 / tcfg.get().playlist_items_per_page*1.0);

			if (page_prev != playlist_page)
				httpc::playlist_page_switched = true;
			else
				playlist_page_switched = false;
		}

		if (pc->get_now_playing(pb_item_ptr))
		{
			pb_length = pb_item_ptr->get_length();

			if (pb_playlist == pfc::infinite_size  || pb_item == pfc::infinite_size)
			{
				pb_item_ptr->format_title(NULL, pb_helper1, script_helper1, NULL);
				pb_item_ptr->format_title(NULL, pb_helper2, script_helper2, NULL);
				pb_item_ptr->format_title(NULL, pb_helper3, script_helper3, NULL);
				pb_item_ptr->format_title(NULL, pb_albumart_lookup, script_albumart_lookup, NULL);
			}
			else
			{
				plm->playlist_item_format_title(pb_playlist,pb_item,NULL,pb_helper1,script_helper1,NULL,playback_control::display_level_all);
				plm->playlist_item_format_title(pb_playlist,pb_item,NULL,pb_helper2,script_helper2,NULL,playback_control::display_level_all);
				plm->playlist_item_format_title(pb_playlist,pb_item,NULL,pb_helper3,script_helper3,NULL,playback_control::display_level_all);
				plm->playlist_item_format_title(pb_playlist,pb_item,NULL,pb_albumart_lookup,script_albumart_lookup,NULL,playback_control::display_level_all);
			}

			pb_helper1x = xml_friendly_string(pb_helper1);
			pb_helper2x = xml_friendly_string(pb_helper2);
			pb_helper3x = xml_friendly_string(pb_helper3);

			retrieve_albumart(pb_item_ptr);
		}
		else
		{
			pb_albumart_path_crc32 = tcfg.get().albumart_not_available;
			pb_albumart_status = AS_NO_INFO;
		}

		if (pc->is_paused())
			httpc::pb_state = FOO_PAUSED;
		else if (pc->is_playing())
			httpc::pb_state = FOO_PLAYING;
		else
			httpc::pb_state = FOO_STOPPED;
	}

	void refresh_playlist_view()
	{
		if (/*cfg.main.retrieve_playlist*/ tcfg.get().playlist_items_per_page == 0	|| !httpc::control::is_active() || tcfg.get_count() <= 1)
			return;

		active_playlist_is_undo_available = static_api_ptr_t<playlist_manager>()->activeplaylist_is_undo_available();
		active_playlist_is_redo_available = static_api_ptr_t<playlist_manager>()->activeplaylist_is_redo_available();

		static_api_ptr_t<playlist_manager> plm;

		size_t apl_count = plm->activeplaylist_get_item_count();

		if (apl_count)
		{
			pfc::string8_fast_aggressive track_title;

			t_size item_start = (httpc::playlist_page-1) * tcfg.get().playlist_items_per_page;
			t_size item_end = (httpc::playlist_page-1) * tcfg.get().playlist_items_per_page + tcfg.get().playlist_items_per_page - 1;

			if (item_end > apl_count - 1)
				item_end = apl_count - 1;

			metadb_handle_list pl;
			bit_array_range pl_mask(item_start, item_end-item_start+1);
			plm->activeplaylist_get_items(pl, pl_mask);

			size_t pl_count = pl.get_count();

			playlist_view.set_size(pl_count);

			for (size_t i = 0; i < pl_count; ++i)
				pl[i]->format_title(NULL, playlist_view[i].title, script_playlist_row, NULL);
		}
		else
			playlist_view.set_size(0);

		playlist_item_focused = plm->playlist_get_focus_item(plm->get_active_playlist());

		should_update_playlist = false;
	}

	void refresh_playlist_total_time()
	{
		static_api_ptr_t<playlist_manager> plm;

		metadb_handle_list pl;
		plm->activeplaylist_get_all_items(pl);

		pfc::format_time_ex time_fmt( metadb_handle_list_helper::calc_total_duration(pl) , 0);
		playlist_total_time = time_fmt;

		should_update_playlist_total_time = false;
	}

	void refresh_playback_queue()
	{
		if (tcfg.get().playlist_items_per_page == 0)
			return;

		static_api_ptr_t<playlist_manager> plm;

		size_t pl_count = plm->activeplaylist_get_item_count();
		size_t pl_count2 = playlist_view.get_count();

		queue_total_time = "";

		pfc::list_t<t_playback_queue_item> queue;
		plm->queue_get_contents(queue);

		if (pl_count)
		{
			t_size item_start = (httpc::playlist_page-1) * tcfg.get().playlist_items_per_page;
			t_size item_end = (httpc::playlist_page-1) * tcfg.get().playlist_items_per_page + tcfg.get().playlist_items_per_page;

			for (t_size i = item_start, j = 0; i < pl_count && i < item_end && j < pl_count2; ++i, ++j)
			{
				playlist_entry &ple = playlist_view[j];
				ple.numinqueue.reset();
				ple.title_queue.reset();
				ple.inqueue = false;

				t_size l = queue.get_count();
				for (t_size y = 0; y < l; ++y)
				{
					t_playback_queue_item &qi = queue[y];

					if (qi.m_playlist == active_playlist
					&& qi.m_item == i
					&& qi.m_handle == plm->activeplaylist_get_item_handle(i))
					{
						plm->playlist_item_format_title(active_playlist,qi.m_item,NULL,ple.title_queue,script_playlist_row,NULL,playback_control::display_level_all);

						ple.inqueue = true;

						if (ple.numinqueue.get_length())
							ple.numinqueue << " " << y+1;
						else
							ple.numinqueue << y+1;
					}
				}
			}

			double queue_pb_time = 0;

			t_size l = queue.get_count();
			for (t_size y = 0; y < l; ++y)
				queue_pb_time+=queue[y].m_handle->get_length();

			pfc::format_time_ex time_fmt(queue_pb_time, 0);
			queue_total_time = time_fmt.operator const char *();
		}

		should_update_queue = false;
	}

	void refresh_playlist_list()
	{
		static_api_ptr_t<playlist_manager> plm;

		pfc::string8 pl_str;
		playlist_info pl_i;

		playlist_list.remove_all();

		t_size l = plm->get_playlist_count();

		for (t_size i = 0; i < l; ++i)
			if (plm->playlist_get_name(i, pl_str))
			{
				pl_i.name = xml_friendly_string(pl_str);
				pl_i.items = plm->playlist_get_item_count(i);

				playlist_list.add_item(pl_i);
			}

		active_playlist = plm->get_active_playlist();

		should_update_playlists_list = false;
	}

	size_t is_extension_registered(const char *path)
	{
		size_t path_len = strlen(path);

		if (path_len <= 4)
			return pfc::infinite_size;

		if (path[path_len-1] == '\\')
			return pfc::infinite_size;

		if (strstr(path, "cdda:/") == path)
			for (size_t j = 0; j < httpc::extensions.get_count(); ++j)
				if (_stricmp(httpc::extensions[j], "cda") == 0)
					return j;

		t_size l = httpc::extensions.get_count();
		for (size_t j = 0; j < l; ++j)
		{
			size_t ext_len = strlen(httpc::extensions[j]);

			if (ext_len > 0 && path_len > ext_len)
			{
				if (_stricmp( (path + path_len  - ext_len), httpc::extensions[j]) == 0)
					return j;
			}
		}

		return pfc::infinite_size;
	}

	bool is_protocol_registered(const char *path)
	{
		// hardcoding protocols since there seems to be no way of interrogating fb2k for supported protocols list
		// protocol should be lowercase so no case insensitive comparision is required

		if (strstr(path, "http://") == path)	
			return true;

		return false;
	}

	void get_registered_extensions()
	{
		extensions.remove_all();
		extension_names.remove_all();

		{
			pfc::string8_fastalloc mask;
			pfc::string8_fastalloc name;

			pfc::list_t<pfc::string8> ignored_extensions;
			pfc::splitStringSimple_toList(ignored_extensions, '|', cfg.main.ignored_formats);

			for (t_size i = 0; i < ignored_extensions.get_count(); ++i)
				ignored_extensions[i] = trim(ignored_extensions[i]);

			service_enum_t<input_file_type> e;
			service_ptr_t<input_file_type> ptr;
			if (e.first(ptr)) do {
				unsigned n,m = ptr->get_count();
				for(n=0;n<m;n++)
				{
					mask.reset();
					name.reset();
					if (ptr->get_mask(n,mask) && ptr->get_name(n,name))
					{
						if (!strchr(mask,'|'))
						{
							pfc::string8_fastalloc extension;

							size_t i = 0;

							while(i < mask.get_length())
							{
								if (mask[i] != '*' && mask[i] != '.' && mask[i] != ';')
									extension.add_char(mask[i]);

								if (mask[i] == ';' || i == mask.get_length() - 1)
								{
									bool ignored = false;

									for (t_size j = 0; j < ignored_extensions.get_count(); ++j)
										if (pfc::stringCompareCaseInsensitive(extension, ignored_extensions[j]) == 0)
										{
											ignored = true;
											break;
										}

									if (!ignored)
									{
										extensions.add_item(pfc::string_simple(extension));
										extension_names.add_item(pfc::string_simple(name));
									}

									extension.reset();
								}

								++i;
							}
						}
					}
				}
			} while(e.next(ptr));
		}

		pfc::list_t<pfc::string8> extra_extensions;
		pfc::splitStringSimple_toList(extra_extensions, '|', cfg.main.extra_formats);
		pfc::string8 ext;
		for (t_size i = 0; i < extra_extensions.get_count(); ++i)
		{
			ext = trim(extra_extensions[i]);

			if (ext.get_length())
			{
				extensions.add_item(ext);
				extension_names.add_item("Extra format");
			}
		}

		service_enum_t<playlist_loader> e;
		service_ptr_t<playlist_loader> ptr;
		if (e.first(ptr)) do
		{
			extensions.add_item(ptr->get_extension());
			extension_names.add_item("Playlist");
		} while(e.next(ptr));

		// build restrict mask
		for (size_t i = 0; i < extensions.get_count(); ++i)
		{
			restrict_mask << "*." << extensions[i];
			if (i != extensions.get_count() - 1 )
				restrict_mask << ";";
		}
	}

	void choose_srv_home_dir() 
	{
		pfc::string8 server_root_tmp = cfg.main.server_root;
		remove_trailing_path_separator(server_root_tmp);
		httpc::srv_home_dir = (server_root_tmp.get_length() > 0 ? server_root_tmp : httpc::fb2k_profile_path);

		for (t_size i = 0; i < tcfg.get_count(); ++i)
			if (tcfg.get(i).root.get_length())
					tcfg.get(i).local_path = pfc::string_formatter() << httpc::srv_home_dir << "\\" << tcfg.get(i).root << "\\";
	}

	extern void build_restrict_to_path_list()
	{
		cfg.restrict_to_path_list.remove_all();
		get_list(cfg.main.restrict_to_path, cfg.restrict_to_path_list, '|', true);
	}

	void control_credentials_auth_hash_update()
	{
		pfc::string8 encode_str;
		encode_str << cfg.main.control_credentials_username << ":" << cfg.main.control_credentials_password;
		size_t buf_len = static_cast<int>(sizeof(char) * encode_str.length() * 1.4)+16;
		char *buf = new char[buf_len];
		memset(buf, 0, buf_len);

		WebServerBaseClass::base64encode(const_cast<char *>(encode_str.operator const char *()), buf);

		control_credentials_auth_hash = pfc::string_formatter() << "Basic " << buf;

		delete[] buf;
	}

	void titleformat_compile()
	{
		static_api_ptr_t<titleformat_compiler>()->compile_safe(script_playlist_row,tcfg.get().playlist_row);
		static_api_ptr_t<titleformat_compiler>()->compile_safe(script_helper1,tcfg.get().helper1);
		static_api_ptr_t<titleformat_compiler>()->compile_safe(script_helper2,tcfg.get().helper2);
		static_api_ptr_t<titleformat_compiler>()->compile_safe(script_helper3,tcfg.get().helper3);
		static_api_ptr_t<titleformat_compiler>()->compile_safe(script_albumart_lookup,tcfg.get().albumart);
	}
}
