#include "stdafx.h"
#include "ui.h"
#include "browsefiles.h"
#include "httpserver.h"

char *access_denied_body = "<h1>403 Forbidden</h1><p>You don't have permission to access this document on this server.</p>";
char *content_type_str = "Connection: close\r\nContent-Type: text/html; Charset=UTF-8;";
char *content_no_refresh = "Cache-Control: no-cache\r\nPragma: no-cache\r\nExpires: Mon, 02 Jun 1980 01:02:03 GMT";

mimetypes mime;

namespace httpc {
	namespace control {
		pfc::string8 command_result;
	}
}

void foo_httpserv::log(pfc::string_base &cmd, pfc::string_base &param1, pfc::string_base &param2, pfc::string_base &param3, char *remotehost, pfc::string_base &request_url)
{
	pfc::string8 bufs;

	char buf[32] = { };
	GetDateFormatA(NULL, 0, NULL, "[ dd.MM ", buf,31);
	bufs << buf;
	GetTimeFormatA(NULL, 0, NULL, "HH:mm:ss ] ", buf,31);
	bufs << buf << remotehost << " \t" << request_url;

	if (cmd.get_length()) bufs << "?cmd=" << cmd;
	if (param1.get_length()) bufs << "&param1=" << param1;
	if (param2.get_length()) bufs << "&param2=" << param2;
	if (param3.get_length()) bufs << "&param3=" << param3;

	char *ua = getheader("User-Agent");
	if (ua)
	{
		bufs << "  \t" << ua;
		free(ua);
	}

	foo_info(bufs);
}

size_t foo_httpserv::send_data(char *buf, size_t buf_len)
{
	size_t result_len = buf_len;
	char *result_ptr = buf;
	
	timeval SendTimeout;
	SendTimeout.tv_sec = 0;
	SendTimeout.tv_usec = 250000; // 250 ms
	fd_set fds;
	
	size_t run_result = 1;
	size_t cansend;
	size_t select_res = 0;

	while (((result_len > 0 || bytes_inqueue()) 
		&& run_result > 0 && run_result != 4) && !httpc::control::listener_stop && select_res != SOCKET_ERROR)
	{
		cansend = bytes_cansend();

		if (cansend > result_len)
			cansend = result_len;
	
		if (cansend > 0)
		{
			write_bytes(result_ptr,cansend);
			result_ptr += cansend;
			result_len -= cansend;
    	}
		else
			Sleep(1);

		run_result = run();
	
		FD_ZERO(&fds);
		FD_SET(get_con()->m_socket, &fds);
		select_res = select(0, NULL, &fds, NULL, &SendTimeout);
	}

	if (select_res == SOCKET_ERROR)
		return SEND_SOCKET_ERROR;
	else
	if (result_len > 0)
		return SEND_INCOMPLETE;
	else
		return SEND_OK;
}

size_t foo_httpserv::send_data_zlib(char *buf, size_t length)
{
	z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

	t_size ret, flush, result_len;

	ret = deflateInit2(&strm, Z_CLEVEL, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK)
		return SEND_ZLIB_ERROR;

	size_t buf_len = length;
	char *buf_ptr = buf;
	
	size_t send_result;

	do
	{
		strm.avail_in = Z_CHUNK > buf_len ? buf_len : Z_CHUNK;
		strm.next_in = (unsigned char *) buf_ptr;

		buf_ptr += strm.avail_in;
		buf_len -= strm.avail_in;

		flush = buf_len > 0? Z_NO_FLUSH : Z_FINISH;

		do {
			strm.avail_out = Z_CHUNK;
			strm.next_out = z_outbuf;

			ret = deflate(&strm, flush);

			if (ret == Z_STREAM_ERROR)	return SEND_ZLIB_ERROR;

			result_len = Z_CHUNK - strm.avail_out;
			unsigned char *result_ptr = z_outbuf;

			send_result = send_data((char *)result_ptr, result_len);

		} while (strm.avail_out == 0 && !httpc::control::listener_stop && send_result == SEND_OK);
	} while ((buf_len > 0) && !httpc::control::listener_stop && send_result == SEND_OK);

	strm.next_in = Z_NULL;
	strm.next_out = Z_NULL;
    (void)deflateEnd(&strm);

	if (send_result != SEND_OK)
		return send_result;
	else
	if (buf_len > 0)
		return SEND_INCOMPLETE;
	else
		return SEND_OK;
}

size_t foo_httpserv::send_file(pfc::string_base &filepath)
{
	size_t send_ret = pfc::infinite_size;
	
	HANDLE inFile;

	pfc::stringcvt::string_wide_from_utf8 path_w(filepath);
	inFile = CreateFileW(path_w, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (inFile == INVALID_HANDLE_VALUE) 
		return SEND_IO_ERROR;
	else
	{
		DWORD bytesread;

		do
		{
			if (!ReadFile(inFile, filebuf, FILE_BUFFER, &bytesread, NULL))
			{
				send_ret = SEND_IO_ERROR;
				break;
			}
			
			send_ret = send_data(filebuf, bytesread);

			if (bytesread <= 0)
				break;

		} while (bytesread > 0 && send_ret == SEND_OK && !httpc::control::listener_stop);

		CloseHandle(inFile);
	}

	if (send_ret != SEND_OK)
		return send_ret;
	else
		return SEND_OK;
}

size_t foo_httpserv::send_file_zlib(pfc::string_base &filepath)
{
	size_t send_ret = pfc::infinite_size;
	HANDLE inFile;

	pfc::stringcvt::string_wide_from_utf8 path_w(filepath);
	inFile = CreateFileW(path_w, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (inFile == INVALID_HANDLE_VALUE) 
		return SEND_IO_ERROR;
	else
	{
		DWORD bytesread;

		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;

		t_size flush, result_len;

		if (deflateInit2(&strm, Z_CLEVEL, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
		{
			CloseHandle(inFile);
			return SEND_ZLIB_ERROR;
		}

		do
		{
			if (!ReadFile(inFile, filebuf, FILE_BUFFER, &bytesread, NULL))
			{
				send_ret = SEND_IO_ERROR;
				break;
			}

			strm.avail_in = Z_CHUNK > bytesread ? bytesread : Z_CHUNK;
			strm.next_in = (unsigned char *) filebuf;

			flush = bytesread > 0? Z_NO_FLUSH : Z_FINISH;

			do {
				strm.avail_out = Z_CHUNK;
				strm.next_out = z_outbuf;

				if (deflate(&strm, flush) == Z_STREAM_ERROR)	
				{
					CloseHandle(inFile);
					return SEND_ZLIB_ERROR;
				}

				result_len = Z_CHUNK - strm.avail_out;

				send_ret = send_data((char *)z_outbuf, result_len);

			} while (strm.avail_out == 0 && !httpc::control::listener_stop && send_ret == SEND_OK);
		} while (bytesread > 0 && send_ret == SEND_OK && !httpc::control::listener_stop);

		CloseHandle(inFile);
		strm.next_in = Z_NULL;
		strm.next_out = Z_NULL;
	    (void)deflateEnd(&strm);
	}

	if (send_ret != SEND_OK)
		return send_ret;
	else
		return SEND_OK;
}

bool foo_httpserv::is_gzip_encoding_supported()
{
	bool result = false;
	char *accept_encoding = getheader("Accept-Encoding:");
	if (accept_encoding)
	{
		result = strstr(accept_encoding, "gzip") == 0? false : true;
		free(accept_encoding);
	}

	return result;
}

void foo_httpserv::process_request()
{
	pfc::string8 cmd, param1, param2, param3, request_url, fullpathfile, lastwritetime_str, tmp;
	bool query_root, gzip_encoding_supported, gzip_compressible;
	DWORD content_length = pfc::infinite_size;

	timer.start();
	m_err_str.reset();
	t_size r = 0, ret = WSA_WAIT_TIMEOUT;

	r = run();

	if (r == 0)
	{
		HANDLE handle = WSACreateEvent();
		if (WSAEventSelect(m_con->m_socket, handle, FD_READ | FD_CLOSE | FD_OOB) != 0)
			foo_error("WSAEventSelect in process_request failed");
		
		// sleeping while browser decided do keep stale tcp connection opened
		while (r == 0 && ret != WSA_INVALID_HANDLE && (ret == WSA_WAIT_IO_COMPLETION || ret == WSA_WAIT_TIMEOUT) )
		{
			ret = WSAWaitForMultipleEvents(1, &handle, FALSE, ~1, TRUE);
			WSAResetEvent(handle);
			r = run();
		}

		WSACloseEvent(handle);
	}

	if (r != -1 && r != 0) // got something!
	{
		EnterCriticalSection(&httpc::cs);
		while (r != -1 && r != 4)
		{
			r = run();

			if (r == 0) // wrong call...
			{
				Sleep(1);
				continue;
			}

			if (r == 2)
			{
				cmd = string8_safe(get_request_parm("cmd"));
				url_decode(string8_safe(get_request_file()), request_url);
				url_decode(string8_safe(get_request_parm("param1")), param1);
				url_decode(string8_safe(get_request_parm("param2")), param2);
				url_decode(string8_safe(get_request_parm("param3")), param3);

				if (param3.get_length())
					if (param3.find_first("..\\") != pfc::infinite_size
						|| param3.find_first("../") != pfc::infinite_size)
						param3.reset(); // defend against level-up

				pfc::list_t<pfc::string8> args;
				pfc::splitStringSimple_toList(args, "/", request_url);
				query_root = args.get_count() > 1 ? false : true;

				// get remote IP address
				char remotehost[16] = {0};
				JNL::addr_to_ipstr(get_con()->get_remote(), remotehost, 15);
	
				// remote IP doesn't match the set restriction
				if(cfg.main.control_ip.get_length() && (strcmp(cfg.main.control_ip, "0.0.0.0") != 0) && strcmp(remotehost, cfg.main.control_ip) != 0)
				{
					set_reply_string("HTTP/1.0 403 Forbidden");
					set_reply_header(content_type_str);
					send_reply();
					httpc::ui::generate_html_response(m_err_str, pfc::string_formatter() << "403 Forbidden", pfc::string_formatter() << access_denied_body);
					continue;
				}

				// suggest user to authenticate if authentication is set
				if(cfg.main.control_credentials)
				{
					char *auth = getheader("Authorization:");

					if (auth && strcmp(auth, httpc::control_credentials_auth_hash) !=  0 || !auth )
					{
						if (auth)
							foo_error(pfc::string_formatter() << "AUTH request from " << remotehost << " denied. Auth: " << auth);
	
						free(auth);

						set_reply_string("HTTP/1.0 401 Authorization Required");
						set_reply_header("WWW-Authenticate: Basic realm=\"Authorization\"");
						set_reply_header(content_type_str);
						send_reply();
						httpc::ui::generate_html_response(m_err_str, pfc::string_formatter() << "403 Forbidden", pfc::string_formatter() << access_denied_body);

						continue;
					}

					free(auth);
				}

				if (args.get_count() >= 1)
				{
					pfc::string8 configpath = pfc::string_formatter() << httpc::srv_home_dir << "\\" << args[0] << "\\config";
					if (!tcfg.loadtemplate(configpath, args[0]))
					{
						pfc::string8 body;
						body << "<p>Error loading template configuration file:<br>foo_httpcontrol_data\\" << args[0] << "\\config</p><p>Most likely it happened because the directory are trying to open doesn't contain template, or template files there are incomplete.<p><font color=\"red\">Check foobar2000 console (View/Console) for error details.</font></p>";
						body << "<p><a href=\"/\">Show installed templates</a></p>";
						httpc::ui::generate_html_response(m_err_str, pfc::string_formatter() << "Template error", body);
						set_reply_string("HTTP/1.0 404 Not Found");
						foo_error(pfc::string_formatter() << "couldn't load " << configpath);
						set_reply_header(content_type_str);
						send_reply();
						continue;
					}
				}
				else
				{
					set_reply_string("HTTP/1.0 200 OK");
					httpc::ui::generate_installed_templates_list(m_err_str);
					set_reply_header(content_type_str);
					send_reply();
					continue;
				}

				// proceeding with user request
				{
					bool request_ok = true;

					fullpathfile = pfc::string_formatter() << httpc::srv_home_dir << request_url;

					if (cfg.main.log_access)
						log(cmd, param1, param2, param3, remotehost, request_url);

					if (request_url.get_length() && !query_root)
					{
						// albumart requested
						if (strstr(request_url, "/albumart_") > request_url)
						{
							// albumart from file
							if (httpc::pb_albumart_status & httpc::AS_FILE)
							{
								fullpathfile = httpc::pb_albumart_path;
								request_ok = is_fileexists(fullpathfile, content_length, lastwritetime_str);
							}
							else
							// invalid albumart memory
							if ((httpc::pb_albumart_status & httpc::AS_MEMORY)
								&& httpc::pb_albumart_embedded_ptr.is_empty())
							{
								fullpathfile = pfc::string_formatter() << tcfg.get().local_path << tcfg.get().albumart_not_found;
								request_ok = is_fileexists(fullpathfile, content_length, lastwritetime_str);
							}
							else
							// albumart from memory
							if ((httpc::pb_albumart_status & httpc::AS_MEMORY)
								&& httpc::pb_albumart_embedded_ptr.is_valid())
							{
								request_url.reset();
								is_fileexists(httpc::pb_albumart_embedded_file, content_length, lastwritetime_str);
								content_length = httpc::pb_albumart_embedded_ptr->get_size();
							}
							else
								request_ok = false;
						}
						// requested other file
						else
						{
							request_ok = false;

							if (fullpathfile.find_first("..\\") == pfc::infinite_size
								&& fullpathfile.find_first("../") == pfc::infinite_size  ) // don't do this, dudley :-)
								request_ok = is_fileexists(fullpathfile, content_length, lastwritetime_str);
						}
					}

					if (request_ok)
					{
						pfc::string8_fast_aggressive response_headers("Content-Type: ");
						pfc::string8_fast_aggressive content_type;

						pfc::string8 p3_mime = mime.get_content_type(param3, gzip_compressible);

						if ((request_url.get_length() == 1 || query_root) && !p3_mime.get_length())
								content_type << mime.get_content_type(pfc::string8("x.htm"), gzip_compressible);
						else if ((strstr(request_url, "/albumart_") > request_url) && (httpc::pb_albumart_status & httpc::AS_FILE))
							content_type << mime.get_content_type(httpc::pb_albumart_path, gzip_compressible);
						else
							if (p3_mime.get_length())
								content_type << p3_mime;
						else
						{
							p3_mime = mime.get_content_type(request_url, gzip_compressible);

							if (p3_mime.get_length())
								content_type << p3_mime;
						}

						if (content_type.get_length())
							response_headers << content_type;
						else
							response_headers << "application/octet-stream";

						response_headers << "\r\nConnection: close";

						gzip_encoding_supported = is_gzip_encoding_supported();

						if (gzip_compressible && gzip_encoding_supported && cfg.main.gzip_enable) 
							response_headers << "\r\nContent-Encoding: gzip";

						set_reply_string("HTTP/1.0 200 OK");

						// returning generated response
						if (!request_url.get_length() || query_root) 
						{
							if (!httpc::control::process_command_event)
								httpc::control::process_command_event = CreateEvent(NULL,FALSE,FALSE,NULL);

							ResetEvent(httpc::control::process_command_event);
							httpc::control::process_command(cmd, param1, param2, param3);
							WaitForSingleObject(httpc::control::process_command_event, 15000);

							set_reply_header(const_cast<char *>(response_headers.toString()));
							set_reply_header(content_no_refresh);
						}
						else
						// returning requested file
						{
							set_reply_header(const_cast<char *>(response_headers.toString()));

							if (lastwritetime_str)
							{
								pfc::string8 last_modified_str = pfc::string_formatter() << "Last-Modified: " << lastwritetime_str;
								set_reply_header(const_cast<char *>(last_modified_str.toString()));
							}

							if (!(gzip_compressible && gzip_encoding_supported && cfg.main.gzip_enable))
							{
								pfc::string8 content_length_str;
								content_length_str << "Content-Length: " << (size_t) content_length;
								set_reply_header(const_cast<char *>(content_length_str.toString()) );
							}
						}

						send_reply();
					}
					else
					{
						set_reply_string("HTTP/1.0 404 Not Found");
						set_reply_header(content_type_str);
						set_reply_header(content_no_refresh);
						send_reply();
						httpc::ui::generate_html_response(m_err_str, pfc::string_formatter() << "404 Not Found", pfc::string_formatter() << "<h1>404 Not Found</h1>The requested url " << request_url << " was not found on the server.");
						foo_error(pfc::string_formatter() << "couldn't load " << fullpathfile);
					}
				}
			}
			if (r == 3)
			{
				t_size send_result = pfc_infinite;

				if (m_err_str.get_length()) // sending error msg if available
				{
					send_result = send_data(const_cast<char *>(m_err_str.toString()), m_err_str.get_length());
					m_err_str.reset();
				}
				else // no error msg, proceeding
				{
					pfc::string8_fast_aggressive show = " ";

					// sending albumart
					if (request_url.get_length() == 0 && httpc::pb_albumart_status == httpc::AS_MEMORY && httpc::pb_albumart_embedded_ptr.is_valid())
						send_result = send_data(static_cast<char *>(const_cast<void *>(httpc::pb_albumart_embedded_ptr->get_ptr())), httpc::pb_albumart_embedded_ptr->get_size());
					else
					// returning actual file
					if (request_url.get_length() > 1 && !query_root)
						if (gzip_compressible && gzip_encoding_supported && cfg.main.gzip_enable)
							send_result = send_file_zlib(fullpathfile);
						else
							send_result = send_file(fullpathfile);
					else
					{
						// returning generated response
						if (strcmp(cmd, "Parse") == 0)
						{
							httpc::ui::set_buffer_from_string(param1);
							httpc::ui::parse_buffer_controls(show, timer);
						}
						else if (httpc::control::command_result.get_length())
						{
							show = httpc::control::command_result;
							httpc::control::command_result.reset();
						}
						else
						if (strcmp(cmd, "Browse") == 0)
						{
							if (param1.get_length() == 0)
								param1 = cfg.misc.last_browse_dir;

							if (cfg.restrict_to_path_list.get_count()) // allowing to browse only specified dirs (if any)
    						{
								bool isallowed = false;
		
								if (param1.get_length() > 0)
								{
									pfc::string tmp2(param1.toString());
									t_size l = cfg.restrict_to_path_list.get_count();

									if ((tmp2.indexOf("..\\") == ~0) && (tmp2.indexOf("../") == ~0)) // check for stuff like d:\music\..\..\..\temp
									for (size_t i = 0; i < l; ++i)
									{
	    								pfc::string8_fast_aggressive tmp(param1);
										tmp.truncate(cfg.restrict_to_path_list[i].get_length());
		
										if (pfc::stringCompareCaseInsensitive(tmp, cfg.restrict_to_path_list[i]) == 0)
										{
											isallowed = true;
											break;
    									}
									}
								}
		
								if (! isallowed)
									param1 = " ";
							}
		
    						// strip file name from path if the user clicked on file
							bool is_file = false;
		
							if (param1.length() > 3 && param1[param1.length()-1] != '\\')
							{
								char *stripped_filename = foo_browsefiles::get_path_parent((char *)param1.operator const char *());
								if (stripped_filename)
								{
									param1 = stripped_filename;
									delete[] stripped_filename;
								}
								else // invalid request, displaying root
									param1 = " ";

    							is_file = true;
							}
		
							if (!(strcmp(param3, "NoResponse") == 0))
							{
								httpc::ui::set_buffer_from_file(param3, 2);
								httpc::ui::parse_buffer_browser(param1, show, timer);
							}

							if (param1.get_length() != 0 && !is_file)
								cfg.misc.last_browse_dir = param1;
						}
    					else
						{
							if (!(strcmp(param3, "NoResponse") == 0))
							{
								httpc::ui::set_buffer_from_file(param3, 1);
								httpc::ui::parse_buffer_controls(show, timer);
							}
						}
	    
						if (gzip_compressible && gzip_encoding_supported && cfg.main.gzip_enable)
							send_result = send_data_zlib(const_cast<char *>(show.get_ptr()), show.get_length());
						else
							send_result = send_data(const_cast<char *>(show.get_ptr()), show.get_length());
					}
				}

				if (send_result != 0)
					foo_error(pfc::string_formatter() << "send: " << send_result << " on request: " << request_url);

				r = 4;

				close(0);
			}
		}

		LeaveCriticalSection(&httpc::cs);
	}

}
