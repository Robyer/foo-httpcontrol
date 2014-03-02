#include "stdafx.h"

#include "ui.h"

namespace httpc 
{
namespace ui 
{
	pfc::string8_fast_aggressive tpl;
	pfc::string8_fast_aggressive *result_ptr;
	pfc::string8 browse_path;

	namespace gen 
	{
		funcs_map funcs;
		void (*func )(pfc::string_base &);

		void gen_func_table() 
		{
			funcs["IS_ENQUEUEING"] = &is_enqueueing;
			funcs["IS_PLAYING"] = &is_playing;
			funcs["IS_PAUSED"] = &is_paused;
			funcs["TRACK_POS"] = &item_playing_pos; // to get rid of
			funcs["ITEM_PLAYING_POS"] = &item_playing_pos;
			funcs["LAST_ACTION"] = &last_action;
			funcs["TRACK_LEN"] = &item_playing_len;
			funcs["ITEM_PLAYING_LEN"] = &item_playing_len;
			funcs["TRACK_NUM"] = &playlist_item_playing;
			funcs["PLAYLIST_ITEM_PLAYING"] = &playlist_item_playing;
			funcs["TRACKS_TOTAL"] = &playlist_items_count;
			funcs["PLAYLIST_ITEMS_COUNT"] = &playlist_items_count;
			funcs["PLAYLIST_PLAYING"] = &playlist_playing;
			funcs["PLAYLIST_PLAYING_ITEMS_COUNT"] = &playlist_playing_items_count;
			funcs["STATE_CHANGE"] = &state_change;
			funcs["SEARCH_REQUEST"] = &search_request;
			funcs["VOLUME_OPTION"] = &volume_option;
			funcs["VOLUME"] = &volume;
			funcs["VOLUME_DB"] = &volume_db;
			funcs["PLAYBACK_ORDER_OPTION"] = &playback_order_option;
			funcs["PLAYBACK_ORDER"] = &playback_order;
			funcs["HELPER1"] = &helper1;
			funcs["HELPER2"] = &helper2;
			funcs["HELPER3"] = &helper3;
			funcs["HELPER1X"] = &helper1x;
			funcs["HELPER2X"] = &helper2x;
			funcs["HELPER3X"] = &helper3x;
			funcs["QUERY_OPTION"] = &query_option;
			funcs["QUERY_JS"] = &query_js;
			funcs["QUERY_INFO"] = &query_info;
			funcs["QUERY_INFO_JS"] = &query_info_js;
			funcs["SAC"] = &sac;
			funcs["SAQ"] = &saq;
			funcs["ALBUMART"] = &albumart;
			funcs["PLAYLISTS_OPTION"] = &playlists_option;
			funcs["PLAYLISTS_JS"] = &playlists_js;
			funcs["PLAYLIST_ACTIVE"] = &playlist_active;
			funcs["PLAYLIST_TOTAL_TIME"] = &playlist_total_time;
			funcs["QUEUE_TOTAL_TIME"] = &queue_total_time;
			funcs["PLAYLIST_PAGES"] = &playlist_pages;
			funcs["PLAYLIST_PAGES_SELECT"] = &playlist_pages_select;
			funcs["PLAYLIST_PAGE"] = &playlist_page;
			funcs["PLAYLIST_PAGES_COUNT"] = &playlist_pages_count;
			funcs["PLAYLIST_ITEM_FOCUSED"] = &playlist_item_focused;
			funcs["PLAYLIST_ITEM_PREV_PLAYED"] = &playlist_item_prev_played;
			funcs["PGT"] = &pgt;
			funcs["PLAYLIST"] = &playlist;
			funcs["PLAYLIST_JS"] = &playlist_js;
			funcs["PATH"] = &path;
			funcs["PATH_JS"] = &path_js;
			funcs["PATH_PARENT"] = &path_parent;
			funcs["BROWSER"] = &browser;
			funcs["BROWSER_JS"] = &browser_js;
			funcs["PLAYLIST_ITEMS_PER_PAGE"] = &playlist_items_per_page;
			funcs["PLAYLIST_IS_UNDO_AVAILABLE"] = &playlist_is_undo_available;
			funcs["PLAYLIST_IS_REDO_AVAILABLE"] = &playlist_is_redo_available;
		}

		void is_enqueueing(pfc::string_base &result)
		{
			result = httpc::enqueueing? "1" : "0";
		}

		void is_playing(pfc::string_base &result)
		{
			result = httpc::pb_state == httpc::FOO_PLAYING ? "1" : "0";
		}

		void is_paused(pfc::string_base &result)
		{
			result = httpc::pb_state == httpc::FOO_PAUSED ? "1" : "0";
		}

		void item_playing_pos(pfc::string_base &result)
		{
			result = pfc::format_uint( static_cast<size_t>(floor(httpc::pb_time+0.5)) );
		}

		void last_action(pfc::string_base &result)
		{
			result = pfc::format_uint( httpc::last_action );
		}

		void item_playing_len(pfc::string_base &result)
		{
			result = httpc::pb_length > 0 ? pfc::format_uint( static_cast<size_t>(floor(httpc::pb_length+0.5)) ) : "0"; 
		}

		void playlist_item_playing(pfc::string_base &result)
		{
			result = httpc::pb_item != pfc::infinite_size? pfc::format_uint( static_cast<size_t>(httpc::pb_item) ): "?"; 
		}

		void playlist_items_count(pfc::string_base &result)
		{
			result = httpc::active_playlist_item_count != pfc::infinite_size  ? pfc::format_uint( httpc::active_playlist_item_count ): "?";
		}

		void playlist_playing(pfc::string_base &result)
		{
			result = httpc::pb_playlist != pfc::infinite_size  ? pfc::format_uint( httpc::pb_playlist ) : "?";
		}

		void playlist_playing_items_count(pfc::string_base &result)
		{
			result = httpc::pb_playlist_item_count != pfc::infinite_size  ? pfc::format_uint( httpc::pb_playlist_item_count ) : "?";
		}

		void state_change(pfc::string_base &result)
		{
			result = pfc::format_uint( httpc::state_changed );
			httpc::state_changed = httpc::FSC_NONE;
		}

		void search_request(pfc::string_base &result)
		{
			result = httpc::autoplaylist_request;
		}

		void volume_option(pfc::string_base &result)
		{
			byte vol_scaled = httpc::get_volume();

			result.reset();
			bool selected = false;
			for (int v = 100; v >= 0 ; v -= 5)
			{
				result << "<option value=\"" << v << "\"";
				if (((v - vol_scaled) < 5) && !selected)
				{
					result << " selected";
					selected = true;
				}
				result << ">" << v << "</option>";
			}
		}

		void volume(pfc::string_base &result)
		{
			result = pfc::format_uint( httpc::get_volume() );
		}

		void volume_db(pfc::string_base &result)
		{
			result = pfc::format_uint((size_t)(httpc::volume*(-10)));
		}

		void playback_order_option(pfc::string_base &result)
		{
			size_t pbo = static_api_ptr_t<playlist_manager>()->playback_order_get_active();

			result.reset();
			t_size l = static_api_ptr_t<playlist_manager>()->playback_order_get_count();
			for (t_size i = 0; i < l; ++i)
			{
				result << "<option value=\"" << i << "\"";

				if (i == pbo)
					result << " selected";

				result << ">" << static_api_ptr_t<playlist_manager>()->playback_order_get_name(i) << "</option>";
			}
		}

		void playback_order(pfc::string_base &result)
		{
			result = pfc::format_uint( static_api_ptr_t<playlist_manager>()->playback_order_get_active() );
		}

		void helper1(pfc::string_base &result)
		{
			result = httpc::pb_helper1;
		}

		void helper2(pfc::string_base &result)
		{
			result = httpc::pb_helper2;
		}

		void helper3(pfc::string_base &result)
		{
			result = httpc::pb_helper3;
		}

		void helper1x(pfc::string_base &result)
		{
			result = httpc::pb_helper1x;
		}

		void helper2x(pfc::string_base &result)
		{
			result = httpc::pb_helper2x;
		}

		void helper3x(pfc::string_base &result)
		{
			result = httpc::pb_helper3x;
		}

		void query_option(pfc::string_base &result)
		{
			result.reset();
			result << "<option>" << (httpc::facets::filtered.get_count()==0? "" : string_formatter() << httpc::facets::filtered.get_count() << (httpc::facets::filtered.get_count() == 1? " item" : " items")) << "</option>";
			for (t_size i = 0; i < httpc::facets::filtered.get_count(); ++i)
				result << "<option>" << xml_friendly_string(httpc::facets::filtered[i]) << "</option>";
		}

		void query_js(pfc::string_base &result)
		{
			result = "[";
			pfc::string tmp2;
			t_size c = httpc::facets::filtered.get_count();

			for (t_size i = 0; i < c; ++i)
			{
				if (i != 0)
					result << ",";

				result << "\"" << xml_friendly_string(httpc::facets::filtered[i]) << "\"";
			}

			result << "]";
			
			//static_cast<pfc::string8 *>(&result)->replace_nontext_chars();
		}

		void query_info(pfc::string_base &result)
		{
			result = " ";

			for (t_size i = 0; i < httpc::facets::facet_current; ++i)
			{
				if (i != 0)
					result << " &gt; ";

				result << xml_friendly_string(httpc::facets::selected[i]);
			}
		}

		void query_info_js(pfc::string_base &result)
		{
			// query info
			result = "[";

			for (t_size i = 0; i < httpc::facets::facet_current; ++i)
			{
				if (i != 0)
					result << ",";

				result << "\"" << xml_friendly_string(httpc::facets::selected[i]) << "\"";
			}
			result << "]";

			//static_cast<pfc::string8 *>(&result)->replace_nontext_chars();
		}

		void sac(pfc::string_base &result)
		{
			result = httpc::sac? "checked" : "";
		}

		void saq(pfc::string_base &result)
		{
			result = cfg.misc.stop_after_queue? "checked" : "";
		}

		void albumart(pfc::string_base &result)
		{
			result = httpc::pb_albumart_path_crc32;
		}

		void playlists_option(pfc::string_base &result)
		{
			t_size len = httpc::playlist_list.get_count();
			for (t_size i = 0; i < len ; ++i)
			{
				result << "<option value=\"" << i << "\"";

				if (i == httpc::active_playlist)
					result << " selected=\"selected\"";

				result << ">" << xml_friendly_string(httpc::playlist_list[i].name) << " (" << httpc::playlist_list[i].items << ")" << "</option>";
			}
		}

		void playlists_js(pfc::string_base &result)
		{
			result = "[";
			t_size len = httpc::playlist_list.get_count();
			for (t_size i = 0; i < len ; ++i)
			{
				if (i != 0)
					result << ",";			

				result << "{\"name\":\"" << httpc::playlist_list[i].name << "\",\"count\":\"" << httpc::playlist_list[i].items << "\"}";
			}
			result << "]";
		}

		void playlist_active(pfc::string_base &result)
		{
			result = pfc::format_uint( httpc::active_playlist );
		}

		void playlist_total_time(pfc::string_base &result)
		{
			result = strcmp(httpc::playlist_total_time, "0:00") == 0 ? "" : httpc::playlist_total_time;
		}

		void queue_total_time(pfc::string_base &result)
		{
			result = strcmp(httpc::queue_total_time, "0:00") == 0 ? "" : httpc::queue_total_time;
		}

		void pgt(pfc::string_base &result)
		{
			result = pfc::format_time_ex(httpc::timer.query(),3).operator const char *();
		}

		void playlist_is_undo_available(pfc::string_base &result)
		{
			result = httpc::active_playlist_is_undo_available ? "1" : "0";
		}

		void playlist_is_redo_available(pfc::string_base &result)
		{
			result = httpc::active_playlist_is_redo_available ? "1" : "0";
		}

		void playlist_pages(pfc::string_base &result)
		{
			result.reset();
			if (/*cfg.main.retrieve_playlist*/ tcfg.get().playlist_items_per_page > 0)
			{
				pfc::string8_fast_aggressive playlist_pages;

				httpc::playlist_page = httpc::playlist_page == 0? 1 : httpc::playlist_page;

				// playlist pages switcher
				unsigned int pages_count = (unsigned int) ceil(httpc::active_playlist_item_count * 1.0 / (tcfg.get().playlist_items_per_page* 1.0));

				if (pages_count > 1 /*&& tcfg.get().playlist_items_per_page > 0*/)
				{
					if (httpc::playlist_page != 1)
					{
						result << "<a href=\"/" << tcfg.get().root << "/?cmd=P&param1=1\">" << "First</a> ";
						result << "<a href=\"/" << tcfg.get().root << "/?cmd=P&param1=" << (httpc::playlist_page - 1) << "\">" << "Previous</a> ";
					}
					else
						result  << "First Previous ";

					int page_start, page_end;
					page_start = httpc::playlist_page - cfg_playlist_pages_leftright;
					page_end = httpc::playlist_page + cfg_playlist_pages_leftright;

					if (httpc::playlist_page <= cfg_playlist_pages_leftright)
					{
						page_start = 1;
						page_end = httpc::playlist_page + cfg_playlist_pages_leftright + (cfg_playlist_pages_leftright - httpc::playlist_page) + 1;
					}

					if ((pages_count - httpc::playlist_page) <= cfg_playlist_pages_leftright)
					{
						page_end = pages_count;
						page_start = httpc::playlist_page - cfg_playlist_pages_leftright - (cfg_playlist_pages_leftright - (pages_count - httpc::playlist_page)) + 1;
					}

					if (page_start < 1)	page_start = 1;
					if ((size_t)page_end > pages_count) page_end = pages_count;

					// builds classic playlist pages switcher
					if (pages_count > 1)
					{
						for (size_t i = page_start; i <= (size_t)pages_count && i <= (size_t)page_end; ++i)
						{
							if (i != httpc::playlist_page)
								result << "<a href=\"/" << tcfg.get().root << "/?cmd=P&param1=" << i << "\">" << i << "</a> ";
							else
								result << "<strong>"  << i << "</strong> ";
						}

						if (httpc::playlist_page != pages_count)
						{
							result << "<a href=\"/" << tcfg.get().root << "/?cmd=P&param1=" << (httpc::playlist_page + 1) << "\">" << "Next</a> ";
							result << "<a href=\"/" << tcfg.get().root << "/?cmd=P&param1=" << pages_count << "\">" << "Last</a>";
						}
						else
							result << "Next Last";
					}
				}
			}
		}

		void playlist_pages_select(pfc::string_base &result)
		{
			if (tcfg.get().playlist_items_per_page == 0)
			{
				result.reset();
				return;
			}

			size_t pages_count = (unsigned int) ceil(httpc::active_playlist_item_count * 1.0 / (tcfg.get().playlist_items_per_page * 1.0));
			httpc::playlist_page = httpc::playlist_page == 0? 1 : httpc::playlist_page;

			if (pages_count > 1) {
				for (size_t i = 1; i <= pages_count; ++i)
				{
					if (i != httpc::playlist_page)
						result = pfc::string_formatter() << "<option value=\"" << i << "\">" << i << "</option>";
					else
						result = pfc::string_formatter() << "<option value=\"" << i << "\" selected>" << i << "</option>";
				}
			}
		}

		void playlist_page(pfc::string_base &result)
		{
			if (tcfg.get().playlist_items_per_page == 0)
			{
				result.reset();
				return;
			}

			unsigned int pages_count = (unsigned int) ceil(httpc::active_playlist_item_count * 1.0 / (tcfg.get().playlist_items_per_page* 1.0));

			result = pages_count > 1 ? pfc::format_uint( httpc::playlist_page == 0? 1 : httpc::playlist_page ) : "0";
		}

		void playlist_pages_count(pfc::string_base &result)
		{
			if (tcfg.get().playlist_items_per_page == 0)
			{
				result.reset();
				return;
			}

			result = pfc::format_uint( (unsigned int) ceil(httpc::active_playlist_item_count * 1.0 / (tcfg.get().playlist_items_per_page * 1.0)) );
		}

		void playlist_item_focused(pfc::string_base &result)
		{
			result = httpc::playlist_item_focused != pfc::infinite_size  ? pfc::format_uint( httpc::playlist_item_focused ) : "?";
		}

		void playlist_item_prev_played(pfc::string_base &result)
		{
			result = (httpc::active_playlist == httpc::pb_playlist_prev) && httpc::pb_item_prev != pfc::infinite_size  ? pfc::format_uint( httpc::pb_item_prev ): "";
		}

		void playlist(pfc::string_base &result)
		{
			result.reset();

			if (/*!cfg.main.retrieve_playlist*/ tcfg.get().playlist_items_per_page == 0)
				return;

			pfc::string8_fast_aggressive &result_ref = *result_ptr;

			pfc::string tmp;

			t_size delta = (httpc::playlist_page-1)*tcfg.get().playlist_items_per_page;
			t_size delta2 = httpc::playlist_page*tcfg.get().playlist_items_per_page;

			t_size c = httpc::playlist_view.get_count();

			for (t_size i = 0, i_real = delta; i < c; ++i, ++i_real)
			{
				result_ref << "<tr onclick=\"a('" << i_real << "')\" class=\"";

				if (httpc::playlist_view[i].inqueue)
					result_ref << "queue ";

				if (httpc::pb_item == i_real && httpc::active_playlist == httpc::pb_playlist)
				{
					result_ref << "npr ";
				}
				else
				{
					result_ref << ((i % 2 == 0)? "o" : "e");

					if (httpc::pb_item_prev == i_real && httpc::active_playlist == httpc::pb_playlist_prev)
						result_ref << " prev";
				}
				if (httpc::playlist_item_focused == i_real)
				{
					result_ref << " focus";
				}

				if (httpc::pb_item == i_real && httpc::active_playlist == httpc::pb_playlist)
					result_ref << "\" id=\"nowplaying\">";
				else
					result_ref << "\">";

				if (httpc::playlist_view[i].inqueue && httpc::playlist_view[i].numinqueue.get_length())
					result_ref << httpc::playlist_view[i].title_queue;
				else
					result_ref << httpc::playlist_view[i].title;

				result_ref << "</tr>";
			}
		}

		void playlist_js(pfc::string_base &result)
		{
			result.reset();
			if (/*!cfg.main.retrieve_playlist*/ tcfg.get().playlist_items_per_page == 0)
				return;

			pfc::string8_fast_aggressive &result_ref = *result_ptr;

			result_ref << "[";
			t_size c = httpc::playlist_view.get_count();

			for (t_size i = 0; i < c; ++i)
			{
				if (httpc::playlist_view[i].inqueue && httpc::playlist_view[i].numinqueue.get_length())
					result_ref << httpc::playlist_view[i].title_queue;
				else
					result_ref << httpc::playlist_view[i].title;

				if (i != httpc::playlist_view.get_count() -1)
					result_ref << ",";
			}
			
			result_ref << "]";

//			result_ref.replace_nontext_chars();
		}

		void path(pfc::string_base &result)
		{
			result.reset();

			if (browse_path.get_length())
				url_encode(browse_path, result);
		}

		void path_js(pfc::string_base &result)
		{
			result.reset();

			foo_browsefiles browser;
			pfc::list_t<foo_browsefiles::path_entry_data> path_list;

			browser.buildpath_list(browse_path, path_list);

			result << "[";

			for (unsigned int t = 0; t < path_list.get_count(); ++t)
			{
				pfc::string tmp = path_list[t].url;
				tmp = tmp.replace("\\", "\\\\");

				// Network:\ hack
				pfc::string tmp2 = path_list[t].cmd;
				tmp2 = tmp2.replace(":", "%3A");
				tmp2 = tmp2.replace("\\", "%5C");

				result << " {\"cmd\":\"" << tmp2.ptr() << "\",\"path\": \"" << tmp.ptr() << "\"}";
				if (t != path_list.get_count() - 1)
					result << ",";
			}

			result << " ]";
		}

		void path_parent(pfc::string_base &result)
		{
			result = "%20";

			foo_browsefiles browser;

			if (browse_path.get_length() > 2)
			{
				char *parent = browser.get_path_parent(const_cast<char *>(browse_path.toString()));

				if (parent != NULL)
				{
					pfc::string8 tmp;
					url_encode(parent, tmp);
					delete[] parent;
					result = tmp;
				}
				else
					result = "%20";
			}
		}

		void browser(pfc::string_base &result)
		{
			result.reset();

			pfc::string8_fast_aggressive &result_ref = *result_ptr;

			foo_browsefiles browser;

			if ((browse_path.get_length() > 4) && browse_path[browse_path.get_length()-1] != '\\')
			{
				char *stripped_filename = browser.get_path_parent(const_cast<char *>(browse_path.toString()));
				browser.browse(stripped_filename);
				delete[] stripped_filename;
			}
			else
				browser.browse(browse_path);

			pfc::list_t<foo_browsefiles::path_entry_data> path_list;
			browser.buildpath_list(browse_path, path_list);

			pfc::string8_fast_aggressive three_dots;
			path_parent(three_dots);

			// show current path
			result_ref << "<div class=\"dir\">";
			for (unsigned int t = 0; t < path_list.get_count(); ++t)
				result_ref << "<a href=\"/" << tcfg.get().root << "/?cmd=Browse&amp;param1=" << path_list[t].cmd << "\">" << path_list[t].url << "</a>";
			result_ref << "</div>\n <p>";

			// show three dots pointing to parent level of browsing directory
			result_ref << "<table>";
			if (browse_path.get_length() > 2)
				result_ref << "<tr><td colspan=\"4\"><a href=\"/" << tcfg.get().root << "/?cmd=Browse&amp;param1=" << three_dots << "\">[ .. ]</a></td></tr>";

			unsigned int count = 0;
			// displaying directories first
			pfc::string8_fast_aggressive row;

			t_size l = browser.entries.get_count();
			for (t_size i = 0; i < l; ++i)
				if (browser.entries[i].type == foo_browsefiles::ET_DIR)
				{
					browser_format_row(count++, browser.entries[i], row);
					result_ref << row;
				}

			// the rest goes afterwards
			for (t_size i = 0; i < l; ++i)
				if (browser.entries[i].type != foo_browsefiles::ET_DIR)
				{
					const char *path = browser.entries[i].path;
					
					if (! (cfg.main.hide_nonplayables && httpc::is_extension_registered(path) == pfc::infinite_size)
						|| !cfg.main.hide_nonplayables
						|| (strlen(path) >= 3 
							&& (strstr(path, "\\\\") == path || strstr(path, "Net") == path)) )
					{
						browser_format_row(count++, browser.entries[i], row);
						result_ref << row;
					}
				}

			result_ref << "</table>";
		}

		void browser_js(pfc::string_base &result)
		{
			result.reset();

			pfc::string8_fast_aggressive &result_ref = *result_ptr;

			foo_browsefiles browser;

			if ((browse_path.get_length() > 4) && browse_path[browse_path.get_length()-1] != '\\')
			{
				char *stripped_filename = browser.get_path_parent(const_cast<char *>(browse_path.toString()));
				browser.browse(stripped_filename);
				delete[] stripped_filename;
			}
			else
				browser.browse(browse_path);

			result_ref << "[";

			unsigned int count = 0;
			// displaying directories first
			pfc::string8_fast_aggressive row;

			t_size l = browser.entries.get_count();
			for (t_size i = 0; i < l; ++i)
				if (browser.entries[i].type == foo_browsefiles::ET_DIR)
				{
					if (count != 0)
						result_ref << ",";

					browser_format_row_js(count++, browser.entries[i], row);

					result_ref << row;
				}

			// the rest goes afterwards
			for (t_size i = 0; i < l; ++i)
				if (browser.entries[i].type != foo_browsefiles::ET_DIR)
				{
					const char *path = browser.entries[i].path;

					if (! (cfg.main.hide_nonplayables && httpc::is_extension_registered(path) == pfc::infinite_size)
						|| !cfg.main.hide_nonplayables
						|| (strlen(path) >= 3 
							&& (strstr(path, "\\\\") == path || strstr(path, "Net") == path)) )
					{
						if (count != 0)
							result_ref << ",";

						browser_format_row_js(count++, browser.entries[i], row);
						result_ref << row;
					}
				}

			result_ref << "]";
		}

		void playlist_items_per_page(pfc::string_base &result)
		{
			result = pfc::format_uint( tcfg.get().playlist_items_per_page );
		}

		void browser_format_row(unsigned int count, foo_browsefiles::entry_data &entry, pfc::string8_fast_aggressive &result)
		{
			result.reset();

			pfc::string8 urlencoded_path;

			bool even = (count % 2) != 0 ? true : false;
			int ext_registered = httpc::is_extension_registered(entry.path);

			url_encode(entry.path, urlencoded_path);

			pfc::string8 comment = (ext_registered != pfc::infinite_size  ? httpc::extension_names[ext_registered].get_ptr() : entry.comment);

			result << "<tr ";
			if (entry.type == foo_browsefiles::ET_FILE && ext_registered == pfc::infinite_size)
			{
				result << "class=\"" << (even ? "even" : "odd") << "\">";
				result << "<td class=\"c1\">";
				result << (entry.filename != "" ? entry.filename : entry.path);
				result << "</td>";
			}
			else
			{
				result << "class=\"" << (even ? "reven" : "rodd") << "\">";
				result << "<td class=\"c1\">";
				result << "<a href=\"/" << tcfg.get().root << "/?cmd=Browse&amp;param1=" << urlencoded_path << "\">";
				result << (entry.filename != "" ? entry.filename : entry.path);
				result << "</a></td>";
			}

			result << "<td class=\"c2\">" << entry.size_str << "</td>";
			result << "<td class=\"c3\">" << entry.comment << "</td>";
			result << "<td class=\"c4\">" << entry.time << "</td>";
			result << "</tr>";
		}

		void browser_format_row_js(unsigned int count, foo_browsefiles::entry_data &entry, pfc::string8_fast_aggressive &result)
		{
			result.reset();

			pfc::string8 urlencoded_path;
			pfc::string path = entry.path;
			pfc::string filename = entry.filename;
			url_encode(path.get_ptr(), urlencoded_path);
			path = path.replace("\\", "\\\\");
			filename = filename.replace("\\", "\\\\");

			int ext_registered = httpc::is_extension_registered(entry.path);

			pfc::string8 comment = (ext_registered != pfc::infinite_size  ? httpc::extension_names[ext_registered].get_ptr() : entry.comment);

			result << " { ";
			if (entry.type == foo_browsefiles::ET_FILE && ext_registered == pfc::infinite_size)
				result << "\"p\":\"" << (entry.filename != "" ? filename.get_ptr() : path.get_ptr()) << "\", \"pu\": \"\",";
			else
			{
				result << "\"p\":\"" << (entry.filename != "" ? filename.get_ptr() : path.get_ptr()) << "\",";
				result << "\"pu\":\"" << urlencoded_path << "\",";
			}

			result << "\"fs\":\"" << entry.size_str << "\",";
			result << "\"cm\":\"" << comment << "\",";
			result << "\"ft\":\"" << entry.time << "\"} ";
		}
	}

	void set_buffer_from_file(pfc::string_base &tpl_file, t_size template_id)
	{
		pfc::string8 path, request;

		request << "foo_httpcontrol_data\\" << tcfg.get().root << "\\";

		if (tpl_file.get_length())
		{
			path = pfc::string_formatter() << tcfg.get().local_path << tpl_file;
			request << tpl_file;
		}
		else
		{
			path = tcfg.get().local_path;

			if (template_id & 1)
			{
				static const char *file_name = "foo_httpcontrol_controls_tpl.html";
				path << file_name;
				request << file_name;
			}

			if (template_id & 2)
			{
				static const char *file_name = "foo_httpcontrol_browser_tpl.html";
				path << file_name;
				request << file_name;
			}
		}

		read_file(path, tpl, request);
	}

	void set_buffer_from_string(pfc::string_base &str)
	{
		tpl = str;
	}

	void parse_buffer_controls(pfc::string8_fast_aggressive &res, pfc::hires_timer &timer)
	{
		httpc::timer = timer;
		res.reset();

		t_size len = tpl.get_length();
		char chr;
		pfc::string8_fast_aggressive macro;
				
		pfc::map_t<pfc::string8_fast_aggressive, pfc::string8_fast_aggressive, pfc::comparator_strcmp> macro_values;
		result_ptr = &res;

		for (t_size i = 0; i < len; ++i)
		{
			chr = tpl[i];
			if (chr == '[')
			{
				macro.reset();
				while (++i < len && (chr = tpl[i]) && (isalnum(chr) || chr == '_'))
					macro.add_char(chr);

				if (chr == ']') // got macro keyword
				{
					if (!macro_values.have_item(macro))
					{
						gen::func = gen::funcs[macro];

						if (gen::func)
							(*gen::func)(macro_values[macro]);
						else
							foo_error(pfc::string8("unknown macro: ") << macro);
					}

					res.add_string(macro_values[macro]);
				}
				else
				{
					macro.add_char(chr);
					res.add_char('[');
					res.add_string(macro);
				}
			}
			else
				res.add_char(chr);
		}
	}

	void read_file(const char *path, pfc::string_base &buffer, pfc::string_base &request)
	{
		bool error = false;
		HANDLE inFile;

		pfc::stringcvt::string_wide_from_utf8 path_w(path);

		inFile = CreateFileW(path_w, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

		buffer.reset();

		if (inFile == INVALID_HANDLE_VALUE) 
			error = true;
		else
		{
			buffer.reset();
			static const int bufsize = 16385;
			char buf[bufsize];
			DWORD bytesread;
			BOOL ret = true;

			do
			{
//				ZeroMemory(buf, bufsize);
				ret = ReadFile(inFile, buf, bufsize - 1, &bytesread, NULL);

				if (! ret)
				{
					error = true;
					break;
				}

				buf[bytesread] = '\0';
				buffer << buf;
			} while (bytesread > 0);

			CloseHandle(inFile);
		}

		if (error)
		{
			LPVOID lpMsgBuf;
			DWORD errorcode = GetLastError();
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPTSTR) &lpMsgBuf,
                      0,
                      NULL);

			pfc::string8 errormsg = pfc::stringcvt::string_utf8_from_wide((wchar_t *) lpMsgBuf);
			LocalFree(lpMsgBuf);

			pfc::string8 body;
			body = pfc::string8() <<  "<p>Error reading " << request << "</p><p><font color=\"red\">Check foobar2000 console (View/Console) for error details.</font></p></p><p><a href=\"/" << tcfg.get().root << tcfg.get().url << "\">Open " << tcfg.get().root << " template</a></p>";
			foo_error(pfc::string_formatter() << "couldn't read " << path << " (" << errormsg << (t_size)errorcode << ")");
			generate_html_response(buffer, pfc::string8() << "Invalid request", body);
		}
	}

	void parse_buffer_browser(pfc::string_base &path, pfc::string8_fast_aggressive &res, pfc::hires_timer &timer)
	{
		browse_path = path;

		parse_buffer_controls(res, timer);
	}

	void generate_installed_templates_list(pfc::string_base &response)
	{
		foo_browsefiles browser;
		browser.browse(pfc::string_formatter() << httpc::srv_home_dir << "\\");

		pfc::string8 tmp;
		foobar2000_io::abort_callback_dummy abort_c;

		pfc::string8 body;

		if (browser.entries.get_count())
		{
			body << "<p>Installed templates:</p><ul>";
			for (t_size i = 0; i < browser.entries.get_count(); ++i)
			{
				if (browser.entries[i].type == foo_browsefiles::ET_DIR)
				{
					pfc::string8 path = pfc::string_formatter() << browser.entries[i].path << "config";
					pfc::stringcvt::string_wide_from_utf8 path_w(path);

					if (foobar2000_io::filesystem::g_exists(path, abort_c))
					{
						read_ini_key("url", tmp, path_w);

						if (tmp.get_length())
							body << "<li><a href=\"/" << browser.entries[i].filename << tmp << "\">" << browser.entries[i].filename << "</a></li>";
						else
							body << "<li>" << browser.entries[i].filename << " [config parse error]</li>";
					}
					else
						body << "<li>" << browser.entries[i].filename << " [config not found]</li>";
				}
			}
			body << "</ul>";
		}
		else
		{
			body << "<p>Something went wrong: no templates found!</p><p>Possible explanation: you forgot to get yourself a template, or extracted it into a wrong place.</p><p><font color=\"red\">Check foobar2000 console (View/console) for path where templates are expected to be found.</font></p>";
			foo_error(pfc::string_formatter() << "no templates found in " << httpc::srv_home_dir);
		}

		body << "<p><a href=\"" << forum <<  "\">Get templates!</a></p>";

		generate_html_response(response, pfc::string_formatter() << "Installed templates", body);
	}

	void generate_html_response(pfc::string_base &buffer, pfc::string_base &title, pfc::string_base &body)
	{
		buffer = pfc::string_formatter() << "<html><title>" << title << "</title><body style=\"background-color:light gray; font-family:sans-serif; font-size:1em;\">" << body << "</body></html>";
	}

	}

}