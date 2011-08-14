#ifndef __httpcontrol_state_H__
#define __httpcontrol_state_H__

#define _CRT_SECURE_NO_DEPRECATE 1

#define PLAYLIST_ITEMS_PER_PAGE_MAX 16384

extern bool foo_httpcontrol_isactive();

namespace httpc {
	extern CRITICAL_SECTION cs;

	extern service_ptr_t<titleformat_object> script_playlist_row;
	extern service_ptr_t<titleformat_object> script_helper1;
	extern service_ptr_t<titleformat_object> script_helper2;
	extern service_ptr_t<titleformat_object> script_helper3;
	extern service_ptr_t<titleformat_object> script_albumart_lookup;

	enum foo_state_enum { FOO_STOPPED = 0, FOO_PLAYING = 1, FOO_PAUSED = 2};
	enum foo_last_action { FLC_START = 0, FLC_ENQUEUE = 1, FLC_DEQUEUE = 2, FLC_FOCUS = 3, FLC_REMOVE = 4, FLC_SELECT = 5, FLC_SHIFT = 6};

	struct playlist_entry
	{
		bool	inqueue;
		pfc::string8 numinqueue;
		pfc::string8 title;
		pfc::string8 title_queue;
	};

	struct playlist_info
	{
		pfc::string8 name;	// playlist name
		size_t items;						// playlist item count
	};

	enum foo_state_changed {
		FSC_NONE		= 0,	// nothing changed
		FSC_PLAYBACK	= 1,	// playback state (start, stop, seek, next track etc)
		FSC_PLAYLISTS	= 2,	// playlists state (names, order, removal, addition etc)
		FSC_PLAYLIST	= 4,	// playlist content (items addition, removal, modify, reorder etc)
		FSC_VOLUME		= 8,	// volume
		FSC_PLAYBACK_ORDER = 16	// playback order
	};

	enum albumart_status {
		AS_NO_INFO		= 0,	// no playing item so no albumart info
		AS_FILE			= 1,	// albumart in file
		AS_MEMORY		= 2,	// albumart in memory
		AS_NOT_FOUND	= 4,	// albumart not found anywhere
	};

	extern bool	enqueueing;

	extern size_t		pb_state;
	extern size_t		last_action;
	extern size_t		state_changed;
	extern float		volume;
	extern pfc::string8 pb_helper1;
	extern pfc::string8 pb_helper2;
	extern pfc::string8 pb_helper3;
	extern pfc::string8 pb_helper1x;
	extern pfc::string8 pb_helper2x;
	extern pfc::string8 pb_helper3x;
	extern pfc::string8 pb_albumart_path;
	extern pfc::string8 pb_albumart_path_crc32;
	extern pfc::string8 pb_albumart_lookup;
	extern pfc::string8 pb_albumart_embedded_file;
	extern albumart_status pb_albumart_status;
	extern album_art_data_ptr pb_albumart_embedded_ptr;
	extern size_t		pb_item;
	extern double		pb_length;
	extern double		pb_time;
	extern size_t		pb_playlist;
	extern size_t		pb_playlist_item_count;
	extern size_t		pb_item_prev;
	extern size_t		pb_playlist_prev;
	extern size_t		playlist_item_focused;
	extern size_t		playlist_page;
	extern size_t		active_playlist;
	extern size_t		active_playlist_item_count;
	extern size_t		query_playlist;
	extern bool			playlist_page_switched;
	extern bool			should_focus_on_playing;
	extern bool			should_update_playlist;
	extern bool			should_update_playlist_total_time;
	extern bool			should_update_playlists_list;
	extern bool			should_update_queue;
	extern pfc::list_t<playlist_entry,pfc::alloc_fast_aggressive> playlist_view;
	extern pfc::string_simple	playlist_total_time;
	extern pfc::string_simple	queue_total_time;
	extern bool			sac;
	extern bool			reset_sac;
	extern bool			active_playlist_is_undo_available;
	extern bool			active_playlist_is_redo_available;
	extern pfc::string_simple control_credentials_auth_hash;
	extern pfc::list_t<pfc::string_simple> extensions;
	extern pfc::list_t<pfc::string_simple> extension_names;
	extern pfc::string8	restrict_mask;
	extern pfc::list_t<playlist_info> playlist_list;
	extern pfc::string8 fb2k_profile_path;
	extern pfc::string8 fb2k_path;
	extern pfc::string8 srv_home_dir;
	extern pfc::string_simple autoplaylist_request;

	extern pfc::hires_timer timer;

	extern void enqueue(pfc::list_t<t_size> &list);
	extern void dequeue(pfc::list_t<t_size> &list);
	extern void empty_info();

	extern void refresh_volume(float p_volume = 0);
	extern void set_volume(t_size pecent);
	extern byte get_volume();
	extern void retrieve_albumart(metadb_handle_ptr pb_item_ptr);
	extern void refresh_playing_info();
	extern void refresh_playlist_view();
	extern void refresh_playlist_total_time();
	extern void refresh_playback_queue();
	extern void refresh_playlist_list();

	extern void update_previouslyplayed();
	extern void empty_previouslyplayed();

	extern void get_registered_extensions();
	extern size_t is_extension_registered(const char *path); // infininte if not registered, list index elsewere
	extern bool is_protocol_registered(const char *path);

	extern void choose_srv_home_dir();
	extern void build_restrict_to_path_list();

	extern void control_credentials_auth_hash_update();

	extern void titleformat_compile();
};

#endif /*__httpcontrol_state_H__*/