Asynchronious foo_httpcontrol template
0.72 http://code.google.com/p/foo-httpcontrol/

*** Requirements

	This template is developed to be used in modern desktop browsers and 
	is fully compatible with Opera 10, Google Chrome 4 and Firefox 3.5.
	Internet Exploder 8 seems to work as well.

	Some other browsers might also work, but you never know until you try.

	foobar2000 1.0 and foo_httpcontrol 0.97.15 or more recent versions
	are required.

*** Installation

	Extract archive contents retaining directory structure
	to %APPDATA%\foobar2000\foo_httpcontrol_data\

*** Usage

	Open http://127.0.0.1:8888/ajquery/index.html in your browser.
	(Note that IP address and port is configuration specific and may
	be different in your case.)

	The interface should be intuitive enough as it is by itself, except:

	1. Double clicking playlist item starts playback.

	2. Single clicking playlist item (maybe combined with shift or ctrl)
	selects item(s).

	3. Dragging playlist item or selection moves it within the current
	playlist page.
	
	4. Playlist slider is working as playlist page switcher, drag its
	thumb to change page.

	5. A few hotkeys are available (check command buttons tooltips for
	hotkey values).

	6. Use Shift+PageUp and Shift+PageDown to resize playlist.

	It is recommended to enable "Cursor follows playback" option in 
	foobar2000 Playback menu for more convenient playlists browsing.

	You are free to change template parameters by editing ajquery/config
	file. For example, playlist_items_per_page variable is useful when you
	want to permanently modify playlist page size.

*** Release history 2014

	26 Jul  0.72
		fix:	broken media library search;
		add:	autofocus input on search window open;

	02 Mar  0.71
		add:	Second precision seeking (instead of percent), requires 
			foo_httpcontrol 0.97.15;
		fix:	updated to latest jQuery;

	02 Mar  0.70
		fix:	Wrong handling of parent button in file browser;
		fix:	Playback order drop down-box didn't reflect the actual order;

*** Release history 2012

	10 Nov  0.69
		fix:	added a bit of sanity to file browser dialog: list scroll
		        position is remembered for each visited directory, 
		        improved path navigation;
		fix:	added a bit of sanity to search dialog;
		fix:	fixed SAC & SAQ checkboxes behaviour;
		fix:	albumart initialisation;
		fix:	updated jquery & jquery-ui;

	23 Aug  0.68
		add:	playlist page switch buttons;
		fix:	playlist auto width;

*** Release history 2011

	09 Apr 0.67
		fix:	wrong now playing info when playing first track;
		add:	double clicking playlist switcher focuses now playing item;
		add:	removed search button;

	16 Mar 0.66
		fix:	updated jquery & jquery-ui;
		fix:	updated cover art and favicon to match current fb2k theme;
		fix:	seekbar popup vertical position made constant;

	27 Jan 0.65
		fix:	replaced Shift+[ / Shift+] hotkeys with Shift+PageUp / Shift+PageDown due to international keyboards differences;
		fix:	probably got rid of windows moving along with playlist resize;
		fix:	queue total time / selection total time;

*** Release history 2010

	03 May 0.64
		fix:	extra formats in file browser grayed out;
		fix:	queue total time;
		fix:	weird behaviour with zero playlists;
		fix:	ctrl/shift+click selection behaviour;
	17 Apr 0.63
		add:	shift+[ and shift] resizes playlist height;
		fix:	resize playlist width by dragging right side of seekbar;
		add:	show selection total time;
		add:	clicking album title selects whole album;
		fix:	prefomance improvements;

	10 Apr 0.62
		add:	Internet Exploder 8 compatibility;

	30 Mar 0.61
		fix:	some troubles and bugs;

	27 Mar 0.60
		add:	improved playlist drag & drop;
		add:	some fixes and improvements;
		add:	migrated to latest jquery/jquery ui;

	21 Mar 0.59	
		add:	numerous usability improvements;

	17 Mar 0.58
		add:	"Browse to..." in browser window;
		fix:	Remove playlist always removed first one;

	15 Mar 0.57
		add:	reworked playlist items operation style (selecting and
			applying some commands on selection is possible now,
			along with moving selection within the current
			playlist page) - vakata, thanks for the inspiration!;
		add:	a few hotkeys (check command buttons tooltip for
			hotkey values);
		fix:	non-playable files disappeared in browser;
		fix:	excessive size of non-initialized browser window;

	06 Mar 0.56a
		fix:	logarithmic volume control;
		fix:	excessive size of browser window;

	22 Feb 0.56
		add:	logarithmic volume control;
		add:	mute/unmute button;

	23 Jan 0.55
		fix:	incorrect display of xml special chars in page title;
		fix:	fixes and improvements;
		add:	ability to focus on playing item;

	06 Jan 0.54b
		fix:	missing albumart in Firefox;

	06 Jan 0.54a
		fix:	replaced [HELPERN] with [HELPERNX];
		add:	foo_httpcontrol 0.96.3 or newer is required;

	05 Jan 0.54
		fix:	data retrieval timeouts;
		add:	right mouse button click brings up playlist/playlist items popup menus in Chrome/Firefox etc;
		add:	remembering windows position, size and state between sessions (browser cookies must be enabled);

*** Release history 2009

	29 Dec 0.53
		fix: various fixes and improvements;
		add: proper playlist undo/redo support;

	01 Nov 0.52
		fix: various fixes;

	14 Sep 0.51
		add: queue album to playlist popup menu;
		add: flush queue and queue random to playlists popup menu;
		add: auto install feature;

	06 Sep 0.5
		fix: progressbar behaviour in paused state;
		add: resizeable album art;

	03 Sep 0.4
		fix: couldn't perform action on 1st track;

	02 Sep 0.3
		add: usability improvements;
		add: debug info on ajax failure;

	01 Sep 0.2 
		add: playlist operations menu, ctrl+lmb on playlist tab name;
		fix: various fixes;
		
	30 Aug 0.1 Initial release;
