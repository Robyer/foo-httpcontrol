#ifndef __FOO_CONFIG_H__
#define __FOO_CONFIG_H__

#define foo_name "foo_httpcontrol"
#define forum "http://www.hydrogenaudio.org/forums/index.php?showtopic=62218"

#include "state.h"
#include "resource.h"

//externs
namespace httpc{
	namespace control {
		extern volatile bool listener_started;
		extern volatile bool listener_error;
		extern volatile bool listener_stop;
		extern volatile int running_threads;
		extern pfc::string8 listener_info;
		extern void set_active(bool activate);
		extern bool is_active();
	}
}

// {E7E34E73-F17E-4c30-8FBE-435C649060C5}
static const GUID guid_preferences_page1 = { 0xe7e34e73, 0xf17e, 0x4c30, { 0x8f, 0xbe, 0x43, 0x5c, 0x64, 0x90, 0x60, 0xc5 } };
// {21CB8CF4-0332-4d57-9740-C8443E2B873B} 
static const GUID guid_preferences_page2 = { 0x21cb8cf4, 0x332, 0x4d57, { 0x97, 0x40, 0xc8, 0x44, 0x3e, 0x2b, 0x87, 0x3b } };

class config_main {
public:
	pfc::string8	ip;
	t_size			port;
	bool			startserver;
	bool			albumart_embedded_retrieve;	/* dump completely in future */
	bool			retrieve_playlist;			/* dump completely in future */
	bool			hide_nonplayables;
	bool			log_access;
	pfc::string8	server_root;
	pfc::string8	control_ip;
	bool			control_credentials;
	pfc::string8	control_credentials_username;
	pfc::string8	control_credentials_password;
	pfc::string8	restrict_to_path;
	bool			allow_commandline;			/* dump completely in future */
	bool			stop_after_queue_enable;
	bool			gzip_enable;
	pfc::string8	extra_formats;
	pfc::string8	ignored_formats;

	config_main() { reset(); }
	config_main(const config_main &cfg) { copy(cfg); }
	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort);
	void set_data_raw(stream_reader * p_stream,unsigned p_sizehint,abort_callback & p_abort);
	config_main &operator = (const config_main &cfg) { copy(cfg); return *this; }
	bool operator == (const config_main &c);
	void copy(const config_main &cfg);
	void reset();
};

class config_misc {
public:
	pfc::string8	last_browse_dir;
	pfc::string8	query_playlist_name;
	bool			stop_after_queue;

	config_misc() { reset(); }
	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort);
	void set_data_raw(stream_reader * p_stream,unsigned p_sizehint,abort_callback & p_abort);
	void reset();
};

class config_query {
public:
	pfc::string8	sortpattern;
	pfc::string8	step1;
	pfc::string8	step2;
	pfc::string8	step3;
	bool			sendtodedicated;

	config_query() { reset(); }
	config_query(const config_query &cfg) {	copy(cfg); }
	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort);
	void set_data_raw(stream_reader * p_stream,unsigned p_sizehint,abort_callback & p_abort);
	config_query &operator = (const config_query &cfg) { copy(cfg); return *this; }
	bool operator == (const config_query &c);
	void copy(const config_query &g_c);
	void reset();
};

class config: public cfg_var {
	virtual void get_data_raw(stream_writer * p_stream,abort_callback & p_abort);
	virtual void set_data_raw(stream_reader * p_stream,unsigned p_sizehint,abort_callback & p_abort);

public:
	config_main main;
	config_query query;
	config_misc misc;

	pfc::list_t<pfc::string8> restrict_to_path_list;
	
	config(const GUID & p_guid) : cfg_var(p_guid) { }
};

extern config cfg;

class preferences_page_main : public preferences_page_instance, public CDialogImpl<preferences_page_main>
{
private:
	config_main cfg_main_new;

	void update_credentials();
	void update_server_status();
	void updateDialog();
	bool is_server_root_valid();
	void set_server_root_btn_state(bool state);

public:
    enum { IDD = IDD_TAB1 };

	preferences_page_main(preferences_page_callback::ptr callback) : m_callback(callback) { cfg_main_new = cfg.main; }

	t_uint32 get_state();
	void apply();
	void reset();
	HWND get_wnd() { return m_hWnd; }

    BEGIN_MSG_MAP_EX(preferences_page_main)
        MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_COMMAND(OnCommand)
    END_MSG_MAP()

    BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl);

	void onChanged();

	const preferences_page_callback::ptr m_callback;
};


class preferences_page_main_impl : public preferences_page_impl<preferences_page_main> {
    public:
    const char * get_name() { return "HTTP Control"; }
	GUID get_guid() { return guid_preferences_page1; }
    GUID get_parent_guid() { return preferences_page::guid_tools; }
	bool get_help_url(pfc::string_base & p_out) { p_out = forum; return true; }
};

static preferences_page_factory_t<preferences_page_main_impl> g_preferences_page1_impl_factory;

struct httpc_template {
	pfc::string8 root;
	pfc::string8 url;
	pfc::string8 local_path;
	pfc::string8 playlist_row;
	pfc::string8 helper1;
	pfc::string8 helper2;
	pfc::string8 helper3;
	pfc::string8 albumart;
	pfc::string8 albumart_not_found;
	pfc::string8 albumart_not_available;
	t_size playlist_items_per_page;
	t_size albumart_limit_size;
	bool albumart_prefer_embedded;

	foobar2000_io::t_filetimestamp timestamp;
	httpc_template(): playlist_items_per_page(30) {} ;
};

class template_config{
	pfc::list_t<httpc_template> cfg;
	httpc_template cfg_empty;
	t_size cfg_id;
	bool changed;

public:
	template_config() { /* cfg_id = pfc::infinite_size;*/ cfg += cfg_empty; cfg_id = 0; }

	bool checkrange(t_size c_id) {	if (c_id != pfc::infinite_size  && c_id < cfg.get_count()) return true; else return false; }
	bool checkrange() {	if (cfg_id != pfc::infinite_size  && cfg_id < cfg.get_count()) return true; else return false; }
	bool choose(t_size new_cfg_id) { if (checkrange(new_cfg_id)) { if (cfg_id != new_cfg_id) changed = true; cfg_id = new_cfg_id; return true; } else return false; }
	t_size get_count() { return cfg.get_count(); }
	const t_size get_id() { return cfg_id; }
	void remove(t_size c_id) { if (checkrange(c_id)) { cfg.remove_by_idx(c_id); cfg_id = c_id == 0? c_id : c_id - 1; changed = true; }	}
	void set_size(t_size new_size) { cfg.set_size(new_size); changed = true; }
	bool is_changed() { if (changed) { changed = false; return true; } else return false; }
	t_size find(pfc::string8 &name);
	bool loadtemplate(pfc::string_base &path, pfc::string8 &tpl_name);
	httpc_template &get() {/* if (checkrange(cfg_id))*/ return cfg[cfg_id];/* else return cfg_empty;*/ }
	httpc_template &get(t_size c_id) { /*if (checkrange(c_id)) */return cfg[c_id]; /*else return cfg_empty;*/ }
};

extern template_config tcfg;

class preferences_page_query : public preferences_page_instance, public CDialogImpl<preferences_page_query>
{
private:
	config_query cfg_query_new;

	void updateDialog();
public:
    enum { IDD = IDD_TAB3 };

	preferences_page_query(preferences_page_callback::ptr callback) : m_callback(callback) { cfg_query_new = cfg.query; }

	t_uint32 get_state();
	void apply();
	void reset();
	HWND get_wnd() { return m_hWnd; }

    BEGIN_MSG_MAP_EX(preferences_page_query)
        MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_COMMAND(OnCommand)
    END_MSG_MAP()

    BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl);

    void onChanged();

	const preferences_page_callback::ptr m_callback;
};

class preferences_page_query_impl : public preferences_page_impl<preferences_page_query> {
    public:
    const char * get_name() { return "Media Library"; }
	GUID get_guid() { return guid_preferences_page2; }
    GUID get_parent_guid() { return guid_preferences_page1; }
	bool get_help_url(pfc::string_base & p_out) { p_out = "http://www.hydrogenaudio.org/forums/index.php?showtopic=62218"; return true; }
};

static preferences_page_factory_t<preferences_page_query_impl> g_preferences_page1_query_factory;

class stop_after_qeueue_menu_command : public mainmenu_commands
{
    // number of commands
	virtual t_uint32 get_command_count()
    {
		if (cfg.main.stop_after_queue_enable)
			return 1;
		else
			return 0;
    }

    // All commands are identified by a GUID.
    virtual GUID get_command(t_uint32 p_index)
    {
		if (cfg.main.stop_after_queue_enable)
		{
			static const GUID g_guid_stop_after_queue = { 0xa643b240, 0x70be, 0x4eba, { 0x92, 0xfd, 0xc4, 0x8, 0x82, 0x7b, 0x28, 0xe3 } };

			if (p_index == 0) return g_guid_stop_after_queue;
		}

        return pfc::guid_null;
    }

    virtual void get_name(t_uint32 p_index, pfc::string_base & p_out)
    {
		if (cfg.main.stop_after_queue_enable)
	        if (p_index == 0) p_out = "Stop after queue";
    }

    virtual bool get_description(t_uint32 p_index, pfc::string_base & p_out)
    {
		if (cfg.main.stop_after_queue_enable)
        if (p_index == 0)
        {
            p_out = "Stops playback after completing playback queue.";
            return true;
        }
        return false;
    }

	virtual GUID get_parent() { return mainmenu_groups::playback_etc; }

    // execute command
    virtual void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback)
    {
		if (cfg.main.stop_after_queue_enable && p_index == 0)
			cfg.misc.stop_after_queue = cfg.misc.stop_after_queue ? false : true;
    }

    // check whether the item should be checked
    bool is_checked(t_uint32 p_index)
    {
		if (cfg.main.stop_after_queue_enable)
		if (p_index == 0) return cfg.misc.stop_after_queue;
        return false;
    }

    // The standard version of this command does not support checked or disabled
    // commands, so we use our own version.
    virtual bool get_display(t_uint32 p_index, pfc::string_base & p_text, t_uint32 & p_flags)
    {
		if (cfg.main.stop_after_queue_enable)
		{
	        p_flags = 0;
			if (is_checked(p_index))
			{
	            p_flags |= flag_checked;
			}
			get_name(p_index,p_text);
		}

        return true;
    }

};

static mainmenu_commands_factory_t< stop_after_qeueue_menu_command > saq_menu_command;


#endif /*__FOO_CONFIG_*/