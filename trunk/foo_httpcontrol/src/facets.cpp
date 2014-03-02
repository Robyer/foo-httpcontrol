#include "stdafx.h"

#include "facets.h"

namespace httpc 
{
namespace facets
{
	string8 query[facets_total];
	t_size facet_current;
	pfc::list_t<string8> filtered;
	string8 selected[facets_total];

	void fill_playlist()
	{
		if (!static_api_ptr_t<library_manager>()->is_library_enabled()
			|| facet_current == 0)
			return;

		static_api_ptr_t<autoplaylist_manager> apm;
		static_api_ptr_t<playlist_manager> pm;
		t_size playlist;

		if (cfg.query.sendtodedicated)
			playlist = pm->find_or_create_playlist(cfg.misc.query_playlist_name, pfc::infinite_size);
		else
			playlist = pm->get_active_playlist();

		if (playlist != pfc::infinite_size)
		{
			pm->playlist_undo_backup(playlist);	

			if (apm->is_client_present(playlist))
					apm->remove_client(playlist);

			autoplaylist_client::ptr foo_apl_client_ptr = new service_impl_t<foo_autoplaylist_client>;

			apm->add_client(foo_apl_client_ptr, playlist, autoplaylist_flag_sort);

			if (apm->is_client_present(playlist))
				apm->remove_client(playlist);

			pm->set_active_playlist(playlist);

			httpc::should_update_playlist = true;

			httpc::query_playlist = playlist;
		}
	}

	void filter(t_size which_facet)
	{
		query[0] = cfg.query.step1;
		query[1] = cfg.query.step2;
		query[2] = cfg.query.step3;

		if (!static_api_ptr_t<library_manager>()->is_library_enabled()
			|| which_facet >= facets_total
			|| strlen(query[which_facet]) == 0)
		{
			filtered.remove_all();
			return;
		}

/*
		pfc::hires_timer timer; 
		timer.start();
*/
		list_t<metadb_handle_ptr> library;
		string8_fast_aggressive facet;

		service_ptr_t<titleformat_object> script_query;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(script_query,query[which_facet]);

		static_api_ptr_t<metadb>()->database_lock();

		static_api_ptr_t<library_manager>()->get_all_items(library);

		pfc::map_t<pfc::string8, boolean> items;
		pfc::string8_fast_aggressive var_name;

		t_size lc = library.get_count();
		pfc::chain_list_v2_t<pfc::string8> field_values;
		if (which_facet == 0) // don't have to do extra search
			for (size_t i = 0; i < lc; ++i)
			{
				if (library.get_item(i)->format_title(NULL, facet, script_query, NULL))
				{
					if (facet.find_first(';') == pfc::infinite_size)
						items[facet] = true;
					else
					{
						field_values.remove_all();
						pfc::splitStringSimple_toList(field_values, ";", facet);

						for (pfc::iterator<pfc::string8> walk = field_values.first(); walk.is_valid(); ++walk)
						{
							walk->skip_trailing_char();
							items[*walk] = true;
						}
					}
				}
			}
		else
		{
			t_size j;
			bool acceptable;

			list_t<service_ptr_t<titleformat_object>> scripts;
			scripts.set_count(which_facet+1);
			for (j = 0; j <= which_facet; ++j)
				static_api_ptr_t<titleformat_compiler>()->compile_safe(scripts[j],query[j]);

			for (size_t i = 0; i < lc; ++i)
			{
				acceptable = true;
				for (j = 0; j < which_facet && acceptable; ++j)
					if (library.get_item(i)->format_title(NULL, facet, scripts[j], NULL))
					{
						if ((facet.find_first(';') == pfc::infinite_size) && (facet != selected[j]))
							acceptable = false;
						else
						{
							field_values.remove_all();
							pfc::splitStringSimple_toList(field_values, ";", facet);
							acceptable = list_find(field_values, selected[j]);
						}
					}
					else
						acceptable = false;

				if (acceptable && library.get_item(i)->format_title(NULL, facet, script_query, NULL))
					if (facet.get_length() > 0)
					{
						if (facet.find_first(';') == pfc::infinite_size)
							items[facet] = true;
						else
						{
							field_values.remove_all();
							pfc::splitStringSimple_toList(field_values, ";", facet);

							for (pfc::iterator<pfc::string8> walk = field_values.first(); walk.is_valid(); ++walk)
							{
								walk->skip_trailing_char();
								items[*walk] = true;
							}
						}
					}
			}

			for (j = 0; j <= which_facet; ++j)
				scripts[j].release();
		}

		static_api_ptr_t<metadb>()->database_unlock();

		filtered.remove_all();
		pfc::map_t<string8, boolean>::const_iterator iter;
		for (iter = items.first(); iter.is_valid(); ++iter)
			filtered.add_item(xml_friendly_string(pfc::string8(iter->m_key)));

		filtered.sort_t(sortfunc_natural);

		script_query.release();

		/*
			console::formatter() << "facet                     " << which_facet;
		console::formatter() << "library items             " << library.get_count();
		console::formatter() << "filtered items            " << filtered.get_count();
		console::formatter() << "filtered list in " << pfc::format_time_ex(timer.query(),6);
		timer.start();
		console::formatter() << "filtered list sorted in   " << pfc::format_time_ex(timer.query(),6);

		for (size_t i = 0; i < filtered.get_count(); ++i)
			console::info(filtered.get_item(i));
	*/
	};


	void prev_facet()
	{
		if (facet_current < facets_total)
			selected[facet_current].reset();

		if (facet_current > 0)
		{
			--facet_current;
			selected[facet_current].reset();
		}

		filter(facet_current);
	}

	void next_facet(string8 &request)
	{
		if (facet_current > facets_total - 1)
			facet_current = facets_total - 1;

		selected[facet_current] = request;

		++facet_current;

		filter(facet_current);
	}


	bool list_find(pfc::chain_list_v2_t<pfc::string8> &list, pfc::string_base &str)
	{
		bool acceptable = false;

		for (pfc::iterator<pfc::string8> walk = list.first(); walk.is_valid(); ++walk)
		{
			walk->skip_trailing_char();

			if (*walk == str)
			{
				acceptable = true;
				break;
			}
		}

		return acceptable;
	}


}
}
