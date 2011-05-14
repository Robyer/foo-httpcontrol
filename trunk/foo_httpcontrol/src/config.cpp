#include "stdafx.h"

#include "config.h"

template_config tcfg;

// {555F213A-FBF5-44c5-AFD1-7B2307F92674}
static const GUID guid_cfg = { 0x555f213a, 0xfbf5, 0x44c5, { 0xaf, 0xd1, 0x7b, 0x23, 0x7, 0xf9, 0x26, 0x74 } };
config cfg(guid_cfg);

void config_main::copy(const config_main &cfg)
{
	ip = cfg.ip;
	port = cfg.port;
	startserver = cfg.startserver;
	albumart_embedded_retrieve = cfg.albumart_embedded_retrieve;
	retrieve_playlist = cfg.retrieve_playlist;
	hide_nonplayables = cfg.hide_nonplayables;
	log_access = cfg.log_access;
	server_root = cfg.server_root;
	control_ip = cfg.control_ip;
	control_credentials = cfg.control_credentials;
	control_credentials_username = cfg.control_credentials_username;
	control_credentials_password = cfg.control_credentials_password;
	restrict_to_path = cfg.restrict_to_path;
	allow_commandline = cfg.allow_commandline;
	stop_after_queue_enable = cfg.stop_after_queue_enable;
	gzip_enable = cfg.gzip_enable;
	extra_formats = cfg.extra_formats;
	ignored_formats = cfg.ignored_formats;
}

void config_main::reset()
{
	ip = "0.0.0.0";
	port = 8888;
	startserver = true;
	albumart_embedded_retrieve = false;
	retrieve_playlist = true;
	hide_nonplayables = false;
	log_access = false;
	server_root = "";
	control_ip = "0.0.0.0";
	control_credentials = false;
	control_credentials_username = "";
	control_credentials_password = "";
	restrict_to_path = "";
	allow_commandline = false;
	stop_after_queue_enable = false;
	gzip_enable = false;
	extra_formats = "zip|rar";
	ignored_formats = "";
}

void config_main::get_data_raw(stream_writer * p_stream,abort_callback & p_abort)
{
	p_stream->write_lendian_t(albumart_embedded_retrieve, p_abort);
	p_stream->write_lendian_t(allow_commandline, p_abort);
	p_stream->write_lendian_t(control_credentials, p_abort);
	p_stream->write_string(control_credentials_password, p_abort);
	p_stream->write_string(control_credentials_username, p_abort);
	p_stream->write_string(control_ip, p_abort);
	p_stream->write_lendian_t(hide_nonplayables, p_abort);
	p_stream->write_string(ip, p_abort);
	p_stream->write_lendian_t(log_access, p_abort);
	p_stream->write_lendian_t(port, p_abort);
	p_stream->write_string(restrict_to_path, p_abort);
	p_stream->write_lendian_t(retrieve_playlist, p_abort);
	p_stream->write_string(server_root, p_abort);
	p_stream->write_lendian_t(startserver, p_abort);
	p_stream->write_lendian_t(stop_after_queue_enable, p_abort);
	p_stream->write_lendian_t(gzip_enable, p_abort);
	p_stream->write_string(extra_formats, p_abort);
	p_stream->write_string(ignored_formats, p_abort);
}

void config_main::set_data_raw(stream_reader * p_stream,unsigned p_sizehint,abort_callback & p_abort)
{
	p_stream->read_lendian_t(albumart_embedded_retrieve, p_abort);
	p_stream->read_lendian_t(allow_commandline, p_abort);
	p_stream->read_lendian_t(control_credentials, p_abort);
	p_stream->read_string(control_credentials_password, p_abort);
	p_stream->read_string(control_credentials_username, p_abort);
	p_stream->read_string(control_ip, p_abort);
	p_stream->read_lendian_t(hide_nonplayables, p_abort);
	p_stream->read_string(ip, p_abort);
	p_stream->read_lendian_t(log_access, p_abort);
	p_stream->read_lendian_t(port, p_abort);
	p_stream->read_string(restrict_to_path, p_abort);
	p_stream->read_lendian_t(retrieve_playlist, p_abort);
	p_stream->read_string(server_root, p_abort);
	p_stream->read_lendian_t(startserver, p_abort);
	p_stream->read_lendian_t(stop_after_queue_enable, p_abort);
	p_stream->read_lendian_t(gzip_enable, p_abort);
	p_stream->read_string(extra_formats, p_abort);
	p_stream->read_string(ignored_formats, p_abort);
}

bool config_main::operator == (const config_main &c)
{
	return ((c.ip == ip)
		&&(c.port == port)
		&&(c.startserver == startserver)
		&&(c.albumart_embedded_retrieve == albumart_embedded_retrieve)
		&&(c.retrieve_playlist == retrieve_playlist)
		&&(c.hide_nonplayables == hide_nonplayables)
		&&(c.log_access == log_access)
		&&(c.server_root == server_root)
		&&(c.control_ip == control_ip)
		&&(c.control_credentials == control_credentials)
		&&(c.control_credentials_username == control_credentials_username)
		&&(c.control_credentials_password == control_credentials_password)
		&&(c.restrict_to_path == restrict_to_path)
		&&(c.allow_commandline == allow_commandline)
		&&(c.stop_after_queue_enable == stop_after_queue_enable)
		&&(c.gzip_enable == gzip_enable)
		&&(c.extra_formats == extra_formats)
		&&(c.ignored_formats == ignored_formats));
}

void config_misc::reset()
{
	stop_after_queue = false;
	last_browse_dir = "";
	query_playlist_name = "Query";
}

void config_misc::get_data_raw(stream_writer * p_stream,abort_callback & p_abort)
{
	p_stream->write_lendian_t(stop_after_queue, p_abort);
	p_stream->write_string(last_browse_dir, p_abort);
	p_stream->write_string(query_playlist_name, p_abort);
}

void config_misc::set_data_raw(stream_reader * p_stream,unsigned p_sizehint,abort_callback & p_abort)
{
	p_stream->read_lendian_t(stop_after_queue, p_abort);
	p_stream->read_string(last_browse_dir, p_abort);
	p_stream->read_string(query_playlist_name, p_abort);
}

void config_query::copy(const config_query &cfg)
{
	sortpattern = cfg.sortpattern;
	step1 = cfg.step1;
	step2 = cfg.step2;
	step3 = cfg.step3;
	sendtodedicated = cfg.sendtodedicated;
}

void config_query::reset()
{
	sortpattern = "%date%|%artist%|%album%|%tracknumber%";
	step1 = "%genre%";
	step2 = "%artist%";
	step3 = "%album%";
	sendtodedicated = true;
}

void config_query::get_data_raw(stream_writer * p_stream,abort_callback & p_abort)
{
	p_stream->write_lendian_t(sendtodedicated, p_abort);
	p_stream->write_string(sortpattern, p_abort);
	p_stream->write_string(step1, p_abort);
	p_stream->write_string(step2, p_abort);
	p_stream->write_string(step3, p_abort);
}

void config_query::set_data_raw(stream_reader * p_stream,unsigned p_sizehint,abort_callback & p_abort)
{
	p_stream->read_lendian_t(sendtodedicated, p_abort);
	p_stream->read_string(sortpattern, p_abort);
	p_stream->read_string(step1, p_abort);
	p_stream->read_string(step2, p_abort);
	p_stream->read_string(step3, p_abort);
}

bool config_query::operator == (const config_query &c)
{
	return ((c.sortpattern == sortpattern)
		&& (c.step1 == step1)
		&& (c.step2 == step2)
		&& (c.step3 == step3)
		&& (c.sendtodedicated == sendtodedicated));
}

bool preferences_page_main::is_server_root_valid()
{
	bool result = false;

	if (cfg_main_new.server_root.get_length())
	{
		try
		{
			foobar2000_io::abort_callback_dummy abort_c;
			result = foobar2000_io::filesystem::g_exists(cfg_main_new.server_root, abort_c); // todo tofix g_exists
		}
		catch (...)
		{
		}
	}
	else
		result = true;

	return result;
}

void preferences_page_main::set_server_root_btn_state(bool state)
{
	GetDlgItem(IDC_SERVER_ROOT_BTN).EnableWindow(state);
}

void preferences_page_main::OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	switch(nID | (uNotifyCode << 16))
	{
		case IDC_INTERFACE | (EN_CHANGE << 16):
			cfg_main_new.ip = string_utf8_from_window(wndCtl);
			break;
		case IDC_PORT | (EN_CHANGE << 16):
			cfg_main_new.port = atoi(string_utf8_from_window(GetDlgItem(nID)));
			break;
		case IDC_SERVERSTARTED | BN_CLICKED:
			cfg_main_new.startserver = IsDlgButtonChecked(nID) == BST_CHECKED;
			break;
		case IDC_SERVER_ROOT_BTN | (BN_CLICKED << 16):
			{
				pfc::string8 dir_to_open = cfg_main_new.server_root.get_length() == 0 ? httpc::srv_home_dir : cfg_main_new.server_root;
				pfc::stringcvt::string_wide_from_utf8 path_w(dir_to_open);
				ShellExecute(NULL, _T("explore"), path_w, NULL, NULL, SW_SHOWNORMAL);
			}
			break;
		case IDC_HIDE_NONPLAYABLES | (BN_CLICKED << 16):
			cfg_main_new.hide_nonplayables = IsDlgButtonChecked(nID) == BST_CHECKED;
			break;
		case IDC_LOG_ACCESS | (BN_CLICKED << 16):
			cfg_main_new.log_access = IsDlgButtonChecked(nID) == BST_CHECKED;
			break;
		case IDC_SERVER_ROOT | (EN_CHANGE << 16):
			cfg_main_new.server_root = trim(pfc::string8(string_utf8_from_window(GetDlgItem(nID))));
			set_server_root_btn_state(is_server_root_valid());
			break;
		case IDC_SERVER_ROOT | (EN_KILLFOCUS << 16):
			if (!is_server_root_valid())
			{
				popup_message::g_show(pfc::string_formatter() << "It appears the home directory path you entered, " << cfg_main_new.server_root 
										<< ", is invalid. \n\nHome directory specifies the path foo_httpcontrol uses to look for templates.\nWhen left blank, it defaults to %APPDATA%\\foobar2000\\foo_httpcontrol if foobar2000 is installed in standard mode, or %FB2K_DIRECTORY%\\foo_httpcontrol if foobar2000 is installed in portable mode.", 
										"Home directory error", popup_message::icon_error);
				set_server_root_btn_state(false);
			}
			else
				set_server_root_btn_state(true);

			break;
		case IDC_LAUNCH_BROWSER | (BN_CLICKED << 16):
			{
				pfc::stringcvt::string_wide_from_utf8 path_w(httpc::control::listener_info);
				ShellExecute(NULL, _T("open"), path_w, NULL, NULL, SW_SHOWNORMAL);
			}
			break;
		case IDC_CONTROL_IP | (EN_CHANGE << 16):
			cfg_main_new.control_ip = string_utf8_from_window(GetDlgItem(nID));
			break;
		case IDC_CONTROL_CREDENTIALS | (BN_CLICKED << 16):
			cfg_main_new.control_credentials = IsDlgButtonChecked(nID) == BST_CHECKED;
			update_credentials();
			break;
		case IDC_CONTROL_PASSWORD | (EN_CHANGE << 16):
			cfg_main_new.control_credentials_password = string_utf8_from_window(GetDlgItem(nID));
			cfg_main_new.control_credentials_password = trim(cfg_main_new.control_credentials_password);
			break;
		case IDC_CONTROL_USERNAME | (EN_CHANGE << 16):
			cfg_main_new.control_credentials_username = string_utf8_from_window(GetDlgItem(nID));
			cfg_main_new.control_credentials_username = trim(cfg_main_new.control_credentials_username);
			break;
		case IDC_CONTROL_PATH | (EN_CHANGE << 16):
			cfg_main_new.restrict_to_path = string_utf8_from_window(GetDlgItem(nID));
			cfg_main_new.restrict_to_path = trim(cfg_main_new.restrict_to_path);
			break;
		case IDC_EXTRA_FORMATS | (EN_CHANGE << 16):
			cfg_main_new.extra_formats = string_utf8_from_window(GetDlgItem(nID));
			cfg_main_new.extra_formats = trim(cfg_main_new.extra_formats);
			break;
		case IDC_IGNORED_FORMATS | (EN_CHANGE << 16):
			cfg_main_new.ignored_formats = string_utf8_from_window(GetDlgItem(nID));
			cfg_main_new.ignored_formats = trim(cfg_main_new.ignored_formats);
			break;
		case IDC_STOP_AFTER_QUEUE_ENABLE | (BN_CLICKED << 16):
			cfg_main_new.stop_after_queue_enable = IsDlgButtonChecked(nID) == BST_CHECKED;
			break;
		case IDC_GZIP_ENABLE | (BN_CLICKED << 16):
			cfg_main_new.gzip_enable = IsDlgButtonChecked(nID) == BST_CHECKED;
			break;
		default:;
	}

	onChanged();
}


void preferences_page_main::update_credentials()
{
	GetDlgItem(IDC_CONTROL_PASSWORD).EnableWindow(cfg_main_new.control_credentials);
	GetDlgItem(IDC_CONTROL_USERNAME).EnableWindow(cfg_main_new.control_credentials);
	GetDlgItem(IDC_STATIC_PASSWORD).EnableWindow(cfg_main_new.control_credentials);
	GetDlgItem(IDC_STATIC_USERNAME).EnableWindow(cfg_main_new.control_credentials);
}

void preferences_page_main::update_server_status()
{
	if (!httpc::control::listener_error)
	{
		uSetDlgItemText(m_hWnd, IDC_LAUNCH_BROWSER, pfc::string_formatter() << "Open " << httpc::control::listener_info);
		GetDlgItem(IDC_LAUNCH_BROWSER).EnableWindow(true);
	}
	if (httpc::control::listener_error)
	{
		SetDlgItemText(IDC_LAUNCH_BROWSER, _T("Bind failed"));
		GetDlgItem(IDC_LAUNCH_BROWSER).EnableWindow(false);
	}
	if (!cfg_main_new.startserver)
	{
		SetDlgItemText(IDC_LAUNCH_BROWSER, _T(""));
		GetDlgItem(IDC_LAUNCH_BROWSER).EnableWindow(false);
	}
}

void preferences_page_main::updateDialog()
{
	uSetDlgItemText(m_hWnd,IDC_INTERFACE, cfg_main_new.ip);
	SetDlgItemInt(IDC_PORT, cfg_main_new.port);
	CheckDlgButton(IDC_SERVERSTARTED,cfg_main_new.startserver);
	CheckDlgButton(IDC_HIDE_NONPLAYABLES,cfg_main_new.hide_nonplayables);
	CheckDlgButton(IDC_LOG_ACCESS,cfg_main_new.log_access);
	uSetDlgItemText(m_hWnd,IDC_CONTROL_IP,cfg_main_new.control_ip);
	CheckDlgButton(IDC_CONTROL_CREDENTIALS,cfg_main_new.control_credentials);
	uSetDlgItemText(m_hWnd,IDC_CONTROL_USERNAME,cfg_main_new.control_credentials_username);
	uSetDlgItemText(m_hWnd,IDC_CONTROL_PASSWORD,cfg_main_new.control_credentials_password);
	uSetDlgItemText(m_hWnd,IDC_CONTROL_PATH,cfg_main_new.restrict_to_path);
	uSetDlgItemText(m_hWnd,IDC_EXTRA_FORMATS,cfg_main_new.extra_formats);
	uSetDlgItemText(m_hWnd,IDC_IGNORED_FORMATS,cfg_main_new.ignored_formats);
	uSetDlgItemText(m_hWnd,IDC_SERVER_ROOT,cfg_main_new.server_root);
	CheckDlgButton(IDC_STOP_AFTER_QUEUE_ENABLE,cfg_main_new.stop_after_queue_enable);
	CheckDlgButton(IDC_GZIP_ENABLE,cfg_main_new.gzip_enable);
}

BOOL preferences_page_main::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	updateDialog();
	update_credentials();
	update_server_status();

	set_server_root_btn_state(is_server_root_valid());

	return TRUE;
}

t_uint32 preferences_page_main::get_state() {
    t_uint32 state = preferences_state::resettable;
    if (!(cfg.main == cfg_main_new)) state |= preferences_state::changed;
    return state;
}

void preferences_page_main::reset() {
	cfg_main_new.reset();

	updateDialog();
	update_credentials();
	update_server_status();

	onChanged();
}

void preferences_page_main::apply() {
	if (httpc::control::running_threads == 0)
	{
		cfg.main = cfg_main_new;

		httpc::choose_srv_home_dir();

		httpc::control_credentials_auth_hash_update();

		httpc::get_registered_extensions();

		httpc::build_restrict_to_path_list();

		if (cfg.main.startserver != httpc::control::is_active())
			httpc::control::set_active(cfg.main.startserver);
	
		update_server_status();

		httpc::should_update_playlist = true;
		httpc::should_update_queue = true;

		if (!cfg.main.stop_after_queue_enable)
			cfg.misc.stop_after_queue = false;

		onChanged();
	}
	else
	{
		popup_message::g_show(pfc::string_formatter() << "Client request proceesing is active, settings not saved.\nPlease press Apply when request is processed.", 
			"Warning", popup_message::icon_error);
	}
}

void preferences_page_main::onChanged() {
	m_callback->on_state_changed();
}

void config::get_data_raw(stream_writer * p_stream,abort_callback & p_abort)
{
	cfg.main.get_data_raw(p_stream, p_abort);
	cfg.query.get_data_raw(p_stream, p_abort);
	cfg.misc.get_data_raw(p_stream, p_abort);
}


void config::set_data_raw(stream_reader * p_stream,unsigned p_sizehint,abort_callback & p_abort)
{
	try
	{
		cfg.main.set_data_raw(p_stream, p_sizehint, p_abort);
		cfg.query.set_data_raw(p_stream, p_sizehint, p_abort);
		cfg.misc.set_data_raw(p_stream, p_sizehint, p_abort);
	}
	catch (...)
	{
		throw;
	}
}

t_size template_config::find(pfc::string8 &name)
{
	if (name.get_length())
	{
		t_size l = cfg.get_count();
		for (t_size i = 0; i < l; ++i)
			if (pfc::stringCompareCaseInsensitive(name, get(i).root) == 0)
				return i;
	}

	return pfc::infinite_size;
}

bool template_config::loadtemplate(pfc::string_base &path, pfc::string8 &tpl_name)
{
	if (tpl_name.get_length() == 0)
		return false;

	foobar2000_io::t_filetimestamp timestamp;

	try
	{
		foobar2000_io::t_filestats filestats;
		foobar2000_io::abort_callback_dummy abort_c;
		bool is_writeable;

		foobar2000_io::filesystem::g_get_stats(path, filestats, is_writeable, abort_c);

		timestamp = filestats.m_timestamp;
	}
	catch (...)
	{
		return false;
	}

	t_size n = find(tpl_name);

	if (n != pfc::infinite_size && (timestamp == get(n).timestamp)) // template already loaded and config timestamp didn't change
	{
		choose(n);
		return true;
	}
	
	// template is not loaded or config timestamp changed;

	pfc::string8 tmp;
	pfc::stringcvt::string_wide_from_utf8 path_w(path);

	if (n == pfc::infinite_size )
	{
		set_size(cfg.get_count() + 1);
		n = cfg.get_count() - 1;
	}

	choose(n);

	get().timestamp = timestamp;
	get().root = tpl_name;
	read_ini_key("url", get().url, path_w);
	read_ini_key("playlist_row", get().playlist_row, path_w);
	read_ini_key("helper1", get().helper1, path_w);
	read_ini_key("helper2", get().helper2, path_w);
	read_ini_key("helper3", get().helper3, path_w);
	read_ini_key("albumart", get().albumart, path_w);
	read_ini_key("albumart_not_found", tmp, path_w);
	if (tmp.get_length() && tmp[0] != '/')
		get().albumart_not_found = pfc::string_formatter("/") << tmp;
	else
		get().albumart_not_found = tmp;

	read_ini_key("albumart_not_available", tmp, path_w);
	if (tmp.get_length() && tmp[0] != '/')
		get().albumart_not_available = pfc::string_formatter("/") << tmp;
	else
		get().albumart_not_available = tmp;

	read_ini_key("playlist_items_per_page", tmp, path_w);
	t_size item_count = atoi(tmp);

	get().playlist_items_per_page = item_count > PLAYLIST_ITEMS_PER_PAGE_MAX ? PLAYLIST_ITEMS_PER_PAGE_MAX : item_count;

	read_ini_key("albumart_limit_size", tmp, path_w);
	get().albumart_limit_size = atoi(tmp);

	read_ini_key("albumart_prefer_embedded", tmp, path_w);
	get().albumart_prefer_embedded = atoi(tmp) == 1? true : false;

	changed = true;

	httpc::choose_srv_home_dir();

	return true;
}

void preferences_page_query::updateDialog()
{
	uSetDlgItemText(m_hWnd,IDC_AUTOPLAYLIST_SORT_PATTERN,cfg_query_new.sortpattern);
	uSetDlgItemText(m_hWnd,IDC_AUTOPLAYLIST_QUERY_STEP1,cfg_query_new.step1);
	uSetDlgItemText(m_hWnd,IDC_AUTOPLAYLIST_QUERY_STEP2,cfg_query_new.step2);
	uSetDlgItemText(m_hWnd,IDC_AUTOPLAYLIST_QUERY_STEP3,cfg_query_new.step3);
	CheckDlgButton(IDC_AUTOPLAYLIST_SENDTODEDICATED,cfg_query_new.sendtodedicated);
}

BOOL preferences_page_query::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	updateDialog();

	return TRUE;
}

void preferences_page_query::OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	switch(nID | (uNotifyCode << 16))
	{
	case (EN_CHANGE<<16) | IDC_AUTOPLAYLIST_SORT_PATTERN:
		cfg_query_new.sortpattern = string_utf8_from_window(GetDlgItem(nID));
		cfg_query_new.sortpattern = trim(cfg_query_new.sortpattern);
		break;
	case (EN_CHANGE<<16) | IDC_AUTOPLAYLIST_QUERY_STEP1:
		cfg_query_new.step1 = string_utf8_from_window(GetDlgItem(nID));
		cfg_query_new.step1 = trim(cfg_query_new.step1);
		break;
	case (EN_CHANGE<<16) | IDC_AUTOPLAYLIST_QUERY_STEP2:
		cfg_query_new.step2 = string_utf8_from_window(GetDlgItem(nID));
		cfg_query_new.step2 = trim(cfg_query_new.step2);
		break;
	case (EN_CHANGE<<16) | IDC_AUTOPLAYLIST_QUERY_STEP3:
		cfg_query_new.step3 = string_utf8_from_window(GetDlgItem(nID));
		cfg_query_new.step3 = trim(cfg_query_new.step3);
		break;
	case IDC_AUTOPLAYLIST_SENDTODEDICATED:
		cfg_query_new.sendtodedicated = IsDlgButtonChecked(nID) == BST_CHECKED;
		break;
	default:;
	}

	onChanged();
}


t_uint32 preferences_page_query::get_state() {
    t_uint32 state = preferences_state::resettable;
    if (!(cfg.query == cfg_query_new)) state |= preferences_state::changed;
    return state;
}

void preferences_page_query::reset() {
	cfg_query_new.reset();

	updateDialog();

	onChanged();
}

void preferences_page_query::apply() {
    cfg.query = cfg_query_new;

	onChanged();
}

void preferences_page_query::onChanged() {
	m_callback->on_state_changed();
}
