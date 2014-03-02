#ifndef ___FACETS_H___
#define ___FACETS_H___

using namespace pfc;

namespace httpc 
{
namespace facets
{
	static const t_size facets_total = 3;

	extern string8 query[facets_total];
	extern t_size facet_current;
	extern pfc::list_t<string8> filtered;
	extern string8 selected[facets_total];

/*	static int sortfunc_basic(string8 &p1, string8 &p2) {
		return stricmp_utf8(p1.operator const char *(),p2.operator const char *());
	}*/	

	static int sortfunc_natural(string8 &p1, string8 &p2) {	return compare_natural_utf8(p1, p2); }

	extern void filter(t_size which_facet);
	extern void fill_playlist();
	extern void prev_facet();
	extern void next_facet(string8 &request);

	extern bool list_find(pfc::chain_list_v2_t<pfc::string8> &list, pfc::string_base &str);

	class foo_autoplaylist_client : public autoplaylist_client {
	public:
		foo_autoplaylist_client() {}
		GUID get_guid() { 
			static const GUID apl = { 0x97e0fef2, 0x7c8b, 0x4d7e, { 0xa0, 0x72, 0x5b, 0x25, 0x47, 0xe9, 0x9e, 0xa1 } };
			return apl;
		}

		void filter(metadb_handle_list_cref data, bool * out){
			t_size i, j, l;
			bool acceptable;
			pfc::string8 facet;

			list_t<service_ptr_t<titleformat_object>> scripts;
			scripts.set_count(facet_current+1);
			for (j = 0; j <= facet_current; ++j)
				static_api_ptr_t<titleformat_compiler>()->compile_safe(scripts[j],query[j]);

			pfc::chain_list_v2_t<pfc::string8> field_values;

			l = data.get_count();
			for (i = 0; i < l; ++i)
			{
				acceptable = true;

				for (t_size j = 0; j < facet_current && selected[j].get_length(); ++j)
					if (data.get_item(i)->format_title(NULL, facet, scripts[j], NULL))
					{
						if (facet.find_first(';') == pfc::infinite_size  && facet != selected[j])
						{
							acceptable = false;
							break;
						}
						else
						{
							field_values.remove_all();
							pfc::splitStringSimple_toList(field_values, ';', facet);

							if (!list_find(field_values, selected[j]))
							{
								acceptable = false;
								break;
							}
						}
					}
					else
					{
						acceptable = false;
						break;
					}
				out[i] = acceptable;
			}

			for (j = 0; j <= facet_current; ++j)
				scripts[j].release();
		};

		bool sort(metadb_handle_list_cref p_items,t_size * p_orderbuffer)
		{
			metadb_handle_list_helper::sort_by_format_get_order(p_items, p_orderbuffer, cfg.query.sortpattern, NULL);
			// todo! hack, I suppose
			return false;
		};
		//! Retrieves your configuration data to be used later when re-instantiating your autoplaylist_client after a restart.
		void get_configuration(stream_writer * p_stream,abort_callback & p_abort) { }

		void show_ui(t_size p_source_playlist) { }
	};
}
}

#endif //___FACETS_H___