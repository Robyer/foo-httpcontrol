#ifndef __FOO_UTILS_H__
#define __FOO_UTILS_H__

void foo_info(const char *msg); // push verbose info to console if logging is enabled
void foo_error(const char *msg); // push error to console

void str_to_bitarray(pfc::string_base &str, bit_array_bittable &arr); // convert c-style string to pfc::bitarray
void str_to_list(pfc::string_base &str, pfc::list_t<t_size> &list); // convert c-style string to pfc::list

void url_decode(pfc::string_base &in, pfc::string_base &out); // url-decode c-style string #2
void url_encode(const char *in, pfc::string_base &out);

pfc::string8 trim(pfc::string8 &str);
void get_list(pfc::string8 &str, pfc::list_t<pfc::string8> &list, char separator, bool fix_dir);

long calcCRC(const char *data, t_size data_len);

pfc::string8 xml_friendly_string(pfc::string_base &str);

bool is_fileexists(pfc::string8 &fullpathfile, DWORD &content_length, pfc::string8 &lastwritetime_str);

void format_time(LPFILETIME filetime, pfc::string8 &out, byte format);

bool is_path_separator_my(unsigned c);

void remove_trailing_path_separator(pfc::string8 &path);

pfc::string8 string8_safe(const char *p_string);

void read_ini_key(const char *key, pfc::string_base &value, const wchar_t *path);

wchar_t * utf8_to_wide(const char * in);

int compare_natural_utf8(const char *p1, const char *p2);

#endif /*__FOO_UTILS_H__*/


