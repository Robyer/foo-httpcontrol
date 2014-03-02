#ifndef ___BROWSEFILES_H___
#define ___BROWSEFILES_H___

#define _CRT_SECURE_NO_DEPRECATE 1

#pragma comment(lib, "mpr.lib")

#include <Winnetwk.h>
#include <lm.h>

class foo_browsefiles
{
public:
	enum ENTRY_TYPE
	{
		ET_DRIVE,
		ET_DIR,
		ET_FILE,
		ET_NETWORK,
		ET_NETWORK_PC,
		ET_NETWORK_PC_SHARE,
		ET_ROOT,
		ET_UNKNOWN
	};

	class entry_data
	{
	public:
		ENTRY_TYPE		type;		// type of entry
		pfc::string8	path;		// full path to entry
		pfc::string8	filename;	// entry file name
		pfc::string8	comment;	// entry comment
		t_uint64		size;		// entry size (bytes)
		pfc::string8	size_str;	// human readable size
		pfc::string8	time;		// entry last modificaton time

		entry_data() { };
		entry_data(ENTRY_TYPE _type, pfc::string8 _path, pfc::string8 _filename, int _size, pfc::string8 _size_str, pfc::string8 _time, pfc::string8 _comment)
			: type(_type), path(_path), filename(_filename), size(_size), size_str(_size_str), time(_time), comment(_comment)  {  };
	};

	class path_entry_data
	{
	public:
		pfc::string8	cmd;
		pfc::string8	url;

		path_entry_data() { };
		path_entry_data(pfc::string8 _cmd, pfc::string8 _url)
			: cmd(_cmd), url(_url) {  };
	};

private:
/*	static int sortfunc(entry_data & p1, entry_data & p2) {return stricmp_utf8(p1.path.operator const char *(),p2.path.operator const char *());}*/
	static int sortfunc_natural(entry_data & p1, entry_data & p2) {	return compare_natural_utf8(p1.path, p2.path); }

	ENTRY_TYPE get_path_type(char *);
	void scan_network(ENTRY_TYPE searchfor, char *path, NETRESOURCE *pNr);
public:
	pfc::list_t<entry_data> entries;

	bool browse(pfc::string8 path);
	void buildpath_list(pfc::string8 path, pfc::list_t<path_entry_data> &res);

	static char *get_path_parent(char *path);
	pfc::string8 separate_thousands(DWORD number);
	pfc::string8 format_size(DWORD filesize);
};

#endif //___BROWSEFILES_H___