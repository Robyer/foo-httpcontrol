#ifndef __foo_ui_H__
#define __foo_ui_H__

#define _CRT_SECURE_NO_DEPRECATE 1

#include "facets.h"
#include "browsefiles.h"

namespace httpc
{
namespace ui 
{
	typedef pfc::map_t<const char *, void (* )(pfc::string_base &), pfc::comparator_strcmp> funcs_map;

	extern pfc::string8_fast_aggressive tpl;
	extern pfc::string8_fast_aggressive *result_ptr; // playlist renderers hack to avoid data copying around
	extern pfc::string8 browse_path;

	static const size_t cfg_playlist_pages_leftright = 4;

	extern void read_file(const char *path, pfc::string_base &buffer, pfc::string_base &request);
	
	extern void generate_html_response(pfc::string_base &buffer, pfc::string_base &header, pfc::string_base &body);

	extern void set_buffer_from_file(pfc::string_base &tpl_file, t_size template_id);
	extern void set_buffer_from_string(pfc::string_base &str);
	
	extern void parse_buffer_controls(pfc::string8_fast_aggressive &res, pfc::hires_timer &timer);
	extern void parse_buffer_browser(pfc::string_base &path, pfc::string8_fast_aggressive &res, pfc::hires_timer &timer);	// parse file browser template

	void generate_installed_templates_list(pfc::string_base &response);

	namespace gen {
		extern funcs_map funcs;
		extern void (*func )(pfc::string_base &);

		extern void gen_func_table();
		extern void is_enqueueing(pfc::string_base &result);
		extern void is_playing(pfc::string_base &result);
		extern void is_paused(pfc::string_base &result);
		extern void item_playing_pos(pfc::string_base &result);
		extern void last_action(pfc::string_base &result);
		extern void item_playing_len(pfc::string_base &result);
		extern void playlist_item_playing(pfc::string_base &result);
		extern void playlist_items_count(pfc::string_base &result);
		extern void playlist_playing(pfc::string_base &result);
		extern void playlist_playing_items_count(pfc::string_base &result);
		extern void state_change(pfc::string_base &result);
		extern void search_request(pfc::string_base &result);
		extern void volume_option(pfc::string_base &result);
		extern void volume(pfc::string_base &result);
		extern void volume_db(pfc::string_base &result);
		extern void playback_order_option(pfc::string_base &result);
		extern void playback_order(pfc::string_base &result);
		extern void helper1(pfc::string_base &result);
		extern void helper2(pfc::string_base &result);
		extern void helper3(pfc::string_base &result);
		extern void helper1x(pfc::string_base &result);
		extern void helper2x(pfc::string_base &result);
		extern void helper3x(pfc::string_base &result);
		extern void query_option(pfc::string_base &result);
		extern void query_js(pfc::string_base &result);
		extern void query_info(pfc::string_base &result);
		extern void query_info_js(pfc::string_base &result);
		extern void sac(pfc::string_base &result);
		extern void saq(pfc::string_base &result);
		extern void auto_refresh(pfc::string_base &result);
		extern void albumart(pfc::string_base &result);
		extern void playlists_option(pfc::string_base &result);
		extern void playlists_js(pfc::string_base &result);
		extern void playlist_active(pfc::string_base &result);
		extern void playlist_total_time(pfc::string_base &result);
		extern void queue_total_time(pfc::string_base &result);
		extern void pgt(pfc::string_base &result);
		extern void playlist_pages(pfc::string_base &result);
		extern void playlist_pages_select(pfc::string_base &result);
		extern void playlist_page(pfc::string_base &result);
		extern void playlist_pages_count(pfc::string_base &result);
		extern void playlist_item_focused(pfc::string_base &result);
		extern void playlist_item_prev_played(pfc::string_base &result);
		extern void playlist_js(pfc::string_base &result);
		extern void playlist(pfc::string_base &result);
		extern void playlist_is_undo_available(pfc::string_base &result);
		extern void playlist_is_redo_available(pfc::string_base &result);
		extern void path(pfc::string_base &result);
		extern void path_js(pfc::string_base &result);
		extern void path_parent(pfc::string_base &result);
		extern void browser(pfc::string_base &result);
		extern void browser_js(pfc::string_base &result);
		extern void playlist_items_per_page(pfc::string_base &result);
		extern void browser_format_row(unsigned int count, foo_browsefiles::entry_data &entry, pfc::string8_fast_aggressive &result);
		extern void browser_format_row_js(unsigned int count, foo_browsefiles::entry_data &entry, pfc::string8_fast_aggressive &result);
	}

}
}

#endif /*__foo_ui_H__*/