﻿﻿﻿
var fb;	// fb2k state
var br;	// file browser state
var br_offsets = {}; // directory scroll offsets
var library;
var aa = new Object; // albumart image
var windows_list = {}; // opened dialog windows list
var isworking = false; // true while doing something network related
var settingtabs = false; // true when mangling with playlist tabs
var timeoutid, timeoutid2;
var npid;
var brparams = new Object;
var keypressed = {};
var input_has_focus = false;
var refresh_interval = 1000; // ms,.data retrieval interval, (not supposed to be changed)

var mouse = {
	x: 0,
	y: 0,
	down: false,
	reset: function() { this.down = false; }
}

var drag = {
	start: null,
	end: null,
	dragging: false,
	timeout: null,
	pageshift: 0,
	reset: function() { this.dragging = false; this.pageshift = 0; clearTimeout(this.timeout); }
};

var selection = {
	items: {},
	count: 0,
	lowest: 0,
	highest: 0,
	length: 0,
	calc: function () {
		var count = 0;
		var lowest = fb.playlistItemsCount;
		var highest = 0;
		var length = 0;

		$.each(this.items, function(k,v) {
			if (v)
			{
				k = parseInt(k);
				if (!isNaN(k))
				{
					if (k < lowest) lowest = k;
					if (k > highest) highest = k;

					var
						pr = fb.playlist[k-(fb.playlistPage-1)*fb.playlistItemsPerPage];

					if (pr)
					{
						len = parseInt(pr.ls);
						if (!(isNaN(len)))
							length += len;
					}

					++count;
				}
			}
		});

		this.count = count;
		this.lowest = lowest;
		this.highest = highest;
		this.length = length;
	},
	tostr: function (shift)
	{
		var shift = shift || 0;
		var result = '';

		$.each(this.items, function(k,v) {
			if (v)
			{
				if (result)
					result += ',';
				result += (parseInt(k)+shift);
			}
		});
		return result;
	},
	reset: function () { this.items = {}; this.count = 0; }
}

var tooltip = {
	show: function(text, l, t) {
		$('#tooltip').text(text);

		if (t && l)
			$('#tooltip').css( { position: 'absolute', left: l, top: t } ).show();
	},
	hide: function() {
		$('#tooltip').hide();
	}
}

jQuery.cookie = function(name, value, options) {
/**
 * Cookie plugin
 *
 * Copyright (c) 2006 Klaus Hartl (stilbuero.de)
 * Dual licensed under the MIT and GPL licenses:
 * http://www.opensource.org/licenses/mit-license.php
 * http://www.gnu.org/licenses/gpl.html
 *
 */
    
    if (typeof value != 'undefined') { // name and value given, set cookie
        options = options || {};
        if (value === null) {
            value = '';
            options.expires = -1;
        }
        var expires = '';
        if (options.expires && (typeof options.expires == 'number' || options.expires.toUTCString)) {
            var date;
            if (typeof options.expires == 'number') {
                date = new Date();
                date.setTime(date.getTime() + (options.expires * 24 * 60 * 60 * 1000));
            } else {
                date = options.expires;
            }
            expires = '; expires=' + date.toUTCString(); // use expires attribute, max-age is not supported by IE
        }
        // CAUTION: Needed to parenthesize options.path and options.domain
        // in the following expressions, otherwise they evaluate to undefined
        // in the packed version for some reason...
        var path = options.path ? '; path=' + (options.path) : '';
        var domain = options.domain ? '; domain=' + (options.domain) : '';
        var secure = options.secure ? '; secure' : '';
        document.cookie = [name, '=', encodeURIComponent(value), expires, path, domain, secure].join('');
    } else { // only name given, get cookie
        var cookieValue = null;
        if (document.cookie && document.cookie != '') {
            var cookies = document.cookie.split(';');
            for (var i = 0; i < cookies.length; i++) {
                var cookie = jQuery.trim(cookies[i]);
                // Does this cookie string begin with the name we want?
                if (cookie.substring(0, name.length + 1) == (name + '=')) {
                    cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
                    break;
                }
            }
        }
        return cookieValue;
    }
};

function promptf(msg, value)
{
	input_has_focus = true;
	var result = prompt(msg, value);
	input_has_focus = false;

	return result;
}

/* This script and many more are available free online at
The JavaScript Source!! http://javascript.internet.com
Created by: Justas | http://www.webtoolkit.info/ */
var Url = {

 	// public method for URL encoding
 	encode : function (string) {
 		 return escape(this._utf8_encode(string));
 	},

 	// public method for URL decoding
	 decode : function (string) {
 	 	return this._utf8_decode(unescape(string));
 	},

 	// private method for UTF-8 encoding
 	_utf8_encode : function (string) {
 		if (!string) return '';
  		string = string.replace(/\r\n/g,"\n");
 	 	var utftext = "";

  		for (var n = 0; n < string.length; n++) {
   			var c = string.charCodeAt(n);
   			if (c < 128) {
    				utftext += String.fromCharCode(c);
 			} else if((c > 127) && (c < 2048)) {
  				utftext += String.fromCharCode((c >> 6) | 192);
  				utftext += String.fromCharCode((c & 63) | 128);
 			} else {
  				utftext += String.fromCharCode((c >> 12) | 224);
  				utftext += String.fromCharCode(((c >> 6) & 63) | 128);
 	 			utftext += String.fromCharCode((c & 63) | 128);
 			}
 	}

		return utftext;
	},

 	// private method for UTF-8 decoding
 	_utf8_decode : function (utftext) {
 		if (!utftext) return '';
 		 var string = "";
 		 var i = 0;
 		 var c = c1 = c2 = 0;

  		while ( i < utftext.length ) {
  			 c = utftext.charCodeAt(i);
   			if (c < 128) {
    				string += String.fromCharCode(c);
    				i++;
  			 } else if((c > 191) && (c < 224)) {
 				   c2 = utftext.charCodeAt(i+1);
    				string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
    				i += 2;
  			 } else {
 				   c2 = utftext.charCodeAt(i+1);
    				c3 = utftext.charCodeAt(i+2);
    				string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
   				 i += 3;
 			  }
		  }
		return string;
	 }
}

function getnumfromid(id)
{
	return parseInt(id.substr(1, id.length-1));
}

function gettickcount()
{
	var now = new Date();
	var ticks = now.getTime();
	return ticks;
}

var stripXmlEntities = {
	values: {},
	regexp: null,
	init: function() {
		this.values['&#39;'] = "'";
		this.values['&#92;'] = "\\";
		this.values['&quot;'] = '"';
		this.values['&gt;'] = '>';
		this.values['&lt;'] = '<';
		this.values['&amp;'] = '&';
		var regexp_str = new String;
		jQuery.each(this.values, function(k, v)
		{
			if (regexp_str != '') regexp_str += '|';
			else regexp_str = '(';
			regexp_str += k;
		});
		regexp_str += ')';
		this.regexp = new RegExp(regexp_str, "g");
	},
	perform: function(str) { return str.replace(this.regexp, function (val) { return stripXmlEntities.values[val]; }); }
}
stripXmlEntities.init();

function save_window_to_cookie(wnd, state) // stores state of a window (opened/closed) in the cookie
{
	windows_list[wnd] = state;

	var windows = new Array();

	jQuery.each(windows_list, function(k, v) {
		if (v)
			windows.push(k);
	});

	$.cookie('windows', windows.join('.'), { expires: 365, path: '/'} );
}

function reopen_windows() // restores open windows
{
	if (isworking || !fb || fb && (fb.isPlaying == '1' && fb.helper1 == '') ) // try again if something is loading while we're trying to mangle with windows
	{
		setTimeout('reopen_windows()', 500);
	}
	else
	{
		var cookie = $.cookie('windows');

		if (cookie)
		{
	        	var values = cookie.split('.');

	        	jQuery.each(values, function(k,v) {
	        		switch (v)
	        		{
		        		case 'pbs_dlg':    $('#pbs_btn').click(); break;
		        		case 'aa_pane':    $('#aa_btn').click(); break;
		        		case 'browse_dlg': $('#browse_btn').click(); break;
		        		case 'search_dlg': $('#search_btn').click(); break;
				};
	        	});
		}
	}
}

function restore_playlist_size()
{
	if (isworking || !fb)
		setTimeout('restore_playlist_size()', 250);
	else
	{
		var cookie = $.cookie('pl_size');

		if (cookie)
		{
	       		var pl_size = cookie.split('.');

			if (pl_size.length == 2)
			{
				var diff = $('#tabs').width() - parseInt(pl_size[0]);

		                $('#tabs').width(parseInt(pl_size[0]));
				$('#progressbar').width($('#progressbar').width()-diff);

				if (fb.playlistItemsPerPage != pl_size[1])
					retrievestate('PlaylistItemsPerPage', pl_size[1]);
			}
		}
	}
}

function save_playlist_size(rows)
{
	if (fb)
	{
		rows = rows || fb.playlistItemsPerPage;
		$('#tabs').height('auto');	
		$.cookie('pl_size', [$('#tabs').width(), rows].join('.'), { expires: 365, path: '/'} );
	}
}

function set_playlist_rowcount(rows)
{
	if (!isNaN(rows))
	{
		save_playlist_size(rows);
		$('#tabs').height('auto');	
		retrievestate('PlaylistItemsPerPage', rows);
	}
}

function get_values_from_cookie(name) // array of left, top, width, height
{
	var cookie = $.cookie(name);
	var values = ['auto', 'auto', 'auto', 'auto'];

        if (cookie)
        {
        	var values_cookie = cookie.split('.');
        	for (var i = 0; i < values_cookie.length && i < values.length; ++i)
        	if (!isNaN(values_cookie[i]))
        		values[i] = parseInt(values_cookie[i]);
	}

	return values;
}

function save_dialog_position_to_cookie(name, ui)
{
	var values = get_values_from_cookie(name);

	if (ui.position)
	{
		values[0] = Math.abs(Math.round(ui.position.left));
		values[1] = Math.abs(Math.round(ui.position.top));
	}

	if (ui.size)
	{
		values[2] = Math.abs(Math.round(ui.size.width));
		values[3] = Math.abs(Math.round(ui.size.height));
	}

	$.cookie(name, values.join('.'), { expires: 365, path: '/'} );
}

function save_div_position_to_cookie(name)
{
	var values = get_values_from_cookie(name);
	var div = document.getElementById(name);

	if (div)
	{
		values[0] = Math.round(div.offsetLeft);
		values[1] = Math.round(div.offsetTop);
		values[2] = Math.round(div.offsetWidth);
		values[3] = Math.round(div.offsetHeight);
	}
	
	$.cookie(name, values.join('.'), { expires: 365, path: '/'} );
}


function set_dialog_state(dlg, values)
{
	if (values && (values.length == 4))
	{
		dlg.dialog('option', 'position', [values[0],values[1]]);
		if (values[2] != 'auto')
			dlg.dialog('option', 'width', values[2]);
		if (values[3] != 'auto')
			dlg.dialog('option', 'height', values[3]);
	}
	else
		positiondialog(dlg, mouse.x, mouse.y+15);
}

function blink_dialog(dlg)
{
	if (dlg.dialog('isOpen'))
	{
		dlg.dialog('close');
		dlg.dialog('open');
	}
}


function checkhotkeys(code)
{
	if (selection.count)
	{
		switch (code) {
			case 81: // q
				$('#QueueItems').click();
				break;
			case 87: // w
				$('#DequeueItems').click();
				break;
			case 46: // del
				$('#Del').click();
				break;
			default:
		}
	}

	switch (code) {
		case 85: // u
			$('#Undo').click();
			break;
		case 82: // r
			$('#Redo').click();
			break;
		case 32: // space
			$('#PlayOrPause').click();
			break;
		case 70: // f
			$('#FocusOnPlaying').click();
			break;
		case 83: // s
			$('#SAC').click();
			break;
		case 34: // PageUp
		case 33: // PageDown
			if (keypressed[16] && fb) // shift 
				set_playlist_rowcount(parseInt(fb.playlistItemsPerPage) + parseInt(code == 33?-1:1));
			break;
		default:
	}
}

function keydown(e) { 
	if (e == 16 || e == 17) // shift or ctrl
		keypressed[e] = gettickcount();
        else
	if (!input_has_focus)
		checkhotkeys(e);
}

function keyup(e) { 
	keypressed[e] = 0;
}

function startwork()
{
	isworking = true;
	$("#loading").css("visibility", "visible");
}

function finishwork()
{
	isworking = false;
	$("#loading").css("visibility", "hidden");
}

function pad(str, chr, count) {
	var str_out = str.toString();
	while (str_out.length < count)
		str_out = chr + str_out;
	return str_out;
}

function format_time(time) {
	hours = Math.floor(time / 3600);
	mins = Math.floor((time-hours*3600) / 60);
	secs = pad(time % 60, '0', 2);

	if (hours)
		return [hours, pad(mins, '0', 2), secs].join(':');
	else
		return [mins, secs].join(':');
}

function select_album(pitem)
{
	if (!keypressed[16])
		selection.reset();

	var shift = (fb.playlistPage-1)*fb.playlistItemsPerPage;
	var artist = '';
	var album = '';

	var len = fb.playlist.length;
	for (var i = pitem - shift,k = (fb.playlistPage-1)*fb.playlistItemsPerPage + pitem - shift; i < len; ++i,++k)
	{
		var row = fb.playlist[i];

        	if (artist == '' && album  == '')
        	{
        		artist = row.a;
        		album = row.b;
        	}

        	if (artist == row.a && album == row.b)
        		selection.items[k] = true;
        	else
	        	break;
	}

	updateplaylist();
}

function volume_from_slider(ui)
{
	var dbcommand;
	var msg = '';

 	if (ui.value == 0) // mute
 	{
		msg = 'Mute'; 
		dbcommand = 1000; // -100dB
	}
	else
	{
		var volumedb = (134-ui.value)*0.5*(-1);
		msg = volumedb.toFixed(1) + 'dB'; 
		dbcommand = Math.round(volumedb*(-10));
	}

	command('VolumeDB', dbcommand);

	return msg;

}

function togglestate(selector,condition)
{
	if (condition)
		$(selector).removeClass('ui-state-disabled');
	else
		$(selector).addClass('ui-state-disabled');
}

function updatepreferencesdynamic()
{
	togglestate('#RemovePlaylist,#RenamePlaylist', fb.playlists.length);
	togglestate('#EmptyPlaylist, #QueueRandomItems', fb.playlist.length&&fb.playlists.length);
	togglestate('#Undo', fb.isUndoAvailable=='1');
	togglestate('#Redo', fb.isRedoAvailable=='1');
	togglestate('#FocusOnPlaying', fb.isPlaying=='1' || fb.isPaused=='1');
	togglestate('#QueueItems,#DequeueItems,#Del', selection.count);
	togglestate('#SetFocus', selection.count == 1);
	togglestate('#FlushQueue', fb.queueTotalTime);

	togglestate('#prevpage_btn', fb.playlistPage - 1);
	togglestate('#nextpage_btn', fb.playlistPage == 1 && Math.ceil(fb.playlistItemsCount / fb.playlistItemsPerPage) == 0 ? 0 : fb.playlistPage - Math.ceil(fb.playlistItemsCount / fb.playlistItemsPerPage));

	title = fb.playlistPage != 1 ? 
					'Page '+(fb.playlistPage - 1)+'/'+Math.ceil(fb.playlistItemsCount / fb.playlistItemsPerPage) : ''
	$('#prevpage_btn').attr('title', title);

	title = (fb.playlistPage - Math.floor(fb.playlistItemsCount / fb.playlistItemsPerPage) - 1 != 0)? 
					'Page '+(fb.playlistPage + 1)+'/'+Math.ceil(fb.playlistItemsCount / fb.playlistItemsPerPage) : ''
	$('#nextpage_btn').attr('title', title);
}

function updatebrowser()
{
	var tmp;

	var a = [];
	var len = br.path.length;

	for (var i = 0; i < len; ++i)
	{
		var row = br.path[i];

		a.push(['<a href="#" onclick="br_offsets[\'',br.pathcurrent,'\']=$(\'#browse_dlg\').scrollTop(); retrievebrowserstate(\'', row.cmd, '\');">', row.path.substring(0, row.path.length - 1), '</a>'].join(''));
	}

	$('#browse_path').html(a.join(' &gt; '));

	tmp = '<table class="pl"><tr><td colspan="4">';

	if (br.pathcurrent != '%20')
	{
		brparams.cmdenqueue = 'Browse&param1='+br.pathcurrent+'&param2=EnqueueDir&';
		brparams.cmdenqueuenested = 'Browse&param1='+br.pathcurrent+'&param2=EnqueueDirSubdirs&';

		$('#browse_parent').text('[ .. ]');
		$('#browse_parent').off('click');
		$('#browse_parent').click(function() { br_offsets[br.pathcurrent]=$('#browse_dlg').scrollTop(); retrievebrowserstate(br.parent); });
	}	
	else
	{
		brparams.cmdenqueue = '';
		brparams.cmdenqueuenested = '';
		$('#browse_parent').text('');
		$('#browse_parent').off('click');
		$('#browse_parent').click(function() {  } );
	}

	len = br.browser.length;
	bookmark = false;
	for (var i = 0; i < len; ++i)
	{
		var row = br.browser[i];
		var st = [];

		if (i % 2)
			st.push('br_o');

		if (!row.pu)
			st.push('br_g');

		if (row.pu && !row.cm && row.fs)
			row.cm = 'Extra format';

		if (st.length)
			tmp += ['<tr class="', st.join(' '), '">'].join('');
		else	
			tmp += '<tr>';

		if (row.pu)
			tmp += ['<td style="width:90%"><a href="#" onclick="br_parent_path=\'', row.pu, '\'; br_offsets[\'',br.pathcurrent,'\']=$(\'#browse_dlg\').scrollTop(); retrievebrowserstate(\'', row.pu, '\');">', row.p, '</a></td>'].join('');
		else
			tmp += ['<td style="width:90%">', row.p, '</td>'].join('');

		tmp += ['<td nowrap class="bt-r" style="text-align: right">', row.fs, '</td>',
			'<td nowrap class="bt-r">', row.cm, '</td>',
			'<td nowrap class="bt-r">', row.ft, '</td></tr>'].join('');
	}

	tmp += '</table>';

	$('#browse_table').html(tmp);

	if (br_offsets.hasOwnProperty(br.pathcurrent))
		$('#browse_dlg').scrollTop( br_offsets[br.pathcurrent] );
}

function switchpage(shift)
{
	if (mouse.down && drag.dragging)
		retrievestate('P', fb.playlistPage + shift);
}

function updateplaylistsortable()
{
	$("#pl div[id*='i']").mousedown(function() {
		drag.start = getnumfromid($(this).attr('id'));
		mouse.down = true;
		selection.calc();
	})
	.mouseup(function() {
		if (drag.dragging)
		{
			var shift = drag.end - selection.lowest;

			if (selection.count > 1 && shift > 0)
				shift -= selection.count - 1;

			selection.calc();

			if (shift != 0 & selection.count > 0)
				command('Move', selection.tostr(), shift);
			else
				$("#pl div[id*='i']").removeClass('dragplacer-top').removeClass('dragplacer-bottom');

			selection.reset();
		}

		mouse.reset();
		drag.reset();
	})
	.mousemove(function() {
		if (mouse.down) {
			tooltip.show(selection.tostr());
			if (!drag.dragging)
			{
				drag.dragging = true;
				if (!$('#i'+drag.start).hasClass('pl_selected'))
				{
					$("#pl div[id*='i']").removeClass('pl_selected');
					selection.reset();
					selection.items[drag.start] = true;
					$('#i'+drag.start).addClass('pl_selected')
				}

				selection.calc();
			}

			drag.end = getnumfromid($(this).attr('id'));

			var cl = '';

			clearTimeout(drag.timeout)

			if (mouse.y < $(this).offset().top + $(this).height() / 2)
			{
				if (!$('#i'+(drag.end-1)).hasClass('dragplacer-bottom')
				&& !$('#i'+(drag.end-1)).hasClass('pl_selected'))
					cl = 'dragplacer-top';

				if (drag.end > drag.start)
					drag.end -= 1;

				if (drag.end == (fb.playlistPage - 1)*fb.playlistItemsPerPage && fb.playlistItemsCount > fb.playlistItemsPerPage && fb.playlistPage != 1)
					drag.timeout = setTimeout('switchpage(-1)', 2000);
			}
			else
			{
				if (!$('#i'+(drag.end+1)).hasClass('dragplacer-top')
				&& !$('#i'+(drag.end+1)).hasClass('pl_selected'))
					cl = 'dragplacer-bottom';

				if (drag.end < drag.start && (drag.end >= (fb.playlistPage-1)*fb.playlistItemsPerPage && drag.end <= ((fb.playlistPage-1)*fb.playlistItemsPerPage + fb.playlistItemsPerPage) ))
					drag.end += 1;

				if (drag.end >= ((fb.playlistPage - 1)*fb.playlistItemsPerPage+fb.playlistItemsPerPage - 1) && fb.playlistItemsCount > fb.playlistItemsPerPage && fb.playlistPage != Math.ceil(fb.playlistItemsCount / fb.playlistItemsPerPage))
					drag.timeout = setTimeout('switchpage(1)', 2000);
			}

			if (cl)
			{
				$("#pl div[id*='i']").removeClass('dragplacer-top').removeClass('dragplacer-bottom');
				$(this).addClass(cl);
			}
		}
	});

	$('#pl div, span').disableSelection();
}

function updateselectionstats()
{
        selection.calc();

        var total_time;

	if (selection.count > 1)
		total_time = [format_time(selection.length), '/', fb.playlistTotalTime].join('');
	else
		total_time = fb.playlistTotalTime;

	if (fb.queueTotalTime)
		$('#totaltime').text(['(', fb.queueTotalTime, ') ', total_time].join(''));
	else
		$('#totaltime').text(total_time);

	updatepreferencesdynamic();
}

function updateplaylist()
{
	var shift = (fb.playlistPage-1)*fb.playlistItemsPerPage;
	var npt = fb.playingItem;
	if (!isNaN(npt)) npt = fb.playingItem - shift;
	var npp = fb.playlistPlaying;
	var ap = fb.playlistActive;
	var ppt = fb.prevplayedItem  - shift;
	var ft = fb.focusedItem - shift;

	var st;
	var artist;
	var album;
	var group;

	var ta = ['<div id="pl">'];

	var len = fb.playlist.length;
	for (var i = 0,k = (fb.playlistPage-1)*fb.playlistItemsPerPage; i < len; ++i,++k)
	{
		group = '';
		var row = fb.playlist[i];
		var cl_1 = []; var cl_2 = []; var cl_3 = []; var cl_r = [];

		if (album != row.b || artist != row.a)
		{ //  &mdash;
			group = ['<span class="pl-album" id="n', k, '"> / ', row.a, ' [', row.d, '] ', row.b, "</span>"].join('');

			artist = row.a;
			album = row.b;

			if (i != 0 && ft != i)
			{
				cl_1.push('bbt'); cl_2.push('bbt'); cl_3.push('bbt');
			}
		}

		if (i % 2 == 0)
			cl_r.push('pl_even');

		if (npt == i && ap == npp)
		{
			if (fb.isPlaying == '1')
				row.n = ['<span class="ui-icon ui-icon-play" style="position: absolute; margin-top: -2px;"></span>', row.n].join('');
			else if (fb.isPaused == '1')
				row.n = ['<span class="ui-icon ui-icon-pause" style="position: absolute; margin-top: -2px;"></span>', row.n].join('');
		}

		if (ppt == i)
			cl_r.push('pl_prev');

		if (ft == i)
		{	
			cl_1.push('bbl bbm'); cl_2.push('bbm'); cl_3.push('bbm bbr');
		}
		else
		{
			cl_1.push('bwl bwm'); cl_2.push('bwm '); cl_3.push('bwm bwr');
		}


		if (cl_1.length == 0)
			cl_1.push('bwm');

		ta.push(['<div id="i', k, '" class="pl_row ', cl_r.join(' '), '">',
			'<div class="pl_c1 ', cl_1.join(' '), '">', row.n, '.</div>',
			'<div class="pl_c2 ', cl_2.join(' '), '">', row.t, group, '</div>',
			'<div class="pl_c3 ', cl_3.join(' '), '">', row.l, '</div></div>'].join(''));
	}

	var a = [];
	len = fb.playlistItemsPerPage - fb.playlist.length;
	for (var i = 0; i < len; ++i)
		a.push('<div class="pl_row"><div class="pl_c1">&nbsp;</div><div class="pl_c2">&nbsp;</div><div class="pl_c3">&nbsp;</div></div>');

	ta.push([a.join(''), '</div>'].join(''));

	$('#playlist').html(ta.join(''));

	var playingItem = isNaN(fb.playingItem) ? '?' : fb.playingItem + 1;

	var tmp = '';

	if (fb.isPlaying == "1")
		tmp += ['Playing ', playingItem, ' of ', fb.playlistPlayingItemsCount, ' | ', fb.helper3].join('');
	else
	if (fb.isPaused == "1")
		tmp += ['Paused ', playingItem, ' of ', fb.playlistPlayingItemsCount, ' | ', fb.helper3].join('');
	else
		tmp += 'Stopped ';

	tmp += '<span id="totaltime" style="float: right"></span';
    
	$('#summary').html(tmp);

	var pageslider = $('#pageslider');

	if (fb.playlistItemsCount > fb.playlistItemsPerPage)
	{
		pageslider.slider('option', 'max', Math.ceil(fb.playlistItemsCount*1.0 / fb.playlistItemsPerPage));
		pageslider.slider('value', fb.playlistPage);
		pageslider.slider('enable');
	}
	else
	{
		pageslider.slider('disable');
		pageslider.slider('value', 1);
		pageslider.slider('option', 'max', 1);
	}

	updateplaylistsortable();

	$.each(selection.items, function(k,v) {
		if (v)
			$('#i'+k).addClass('pl_selected');
	});

	$("#pl span[id*='n']").off('click');
	$("#pl span[id*='n']").click(function() {
			select_album(getnumfromid($(this).attr('id')));
		});

	$("#pl div[id*='i']").off('dblclick').off('click');
	$("#pl div[id*='i']").dblclick(function() {
			$("#pl div[id*='i']").removeClass('pl_selected');
			command('Start', getnumfromid($(this).attr('id')));
		})
		.click(function() {
			var i = getnumfromid($(this).attr('id'));

			selection.calc();

			if (keypressed[16] && selection.count) //shift
			{
				$("#pl div[id*='i']").removeClass('pl_selected');

				var start = 0; var end = 0;
				if (i < selection.lowest)
				{
					start = i;
					end = selection.highest;
				}
				if (i > selection.highest)
				{
					start = selection.lowest;
					end = i;
				}

				selection.reset();

				for (var k = start; k <= end; ++k)
				{
					$('#i'+k).addClass('pl_selected');
					selection.items[k] = true;
				}
			}
			else
			if (keypressed[17]) // ctrl
			{
				if (selection.items[i])
				{
					$(this).removeClass('pl_selected');
					selection.items[i] = false;
				}
				else
				{
					$(this).addClass('pl_selected');
					selection.items[i] = true;
				}

			}
			else
			{
				$("#pl div[id*='i']").removeClass('pl_selected');
				selection.reset();
				selection.items[i] = true;
				$(this).addClass('pl_selected');
			}

			updateselectionstats();
		})
		.disableSelection();

	updateselectionstats();
}

function updatenp()
{
	if (!fb) return;

	var pos = parseInt(fb.itemPlayingPos) || 0;
	var len = parseInt(fb.itemPlayingLen) || -1;

	clearTimeout(npid);

	if (pos >= len && pos && len != -1)
	{
		retrievestate();
		return;
	}

	if (len > 0)
	{
		$("#progressbar").progressbar('enable').progressbar('option', 'max', len); // % 10 second resolution
		$("#progressbar").progressbar('value', pos);

		if (pos < len)
		{
			fb.itemPlayingPos = pos + 1;
			$("#playingtime").html("-" + format_time(len-pos));

			if (fb.isPlaying == "1") 
				npid = setTimeout('updatenp()', refresh_interval);
		}
	}
	else
	if (fb.isPlaying != "1" && fb.isPaused != "1")
	{
		$("#progressbar").progressbar('disable').progressbar('value', 0);
		$("#playingtime").html("");
	}

	if (fb.isPlaying == "1" && len == -1)
	{
		$("#progressbar").progressbar('enable').progressbar('value', 0);
		fb.itemPlayingPos = pos + 1;
		$("#playingtime").html(format_time(pos));
		if (pos % 15 == 0)
		{
			retrievestate_schedule(1100, "RefreshPlayingInfo");
		}
		else
			npid = setTimeout('updatenp()', refresh_interval);
	}

}

function updatetabs()
{
	var tc = parseInt($("#tabs >ul >li").size());
	var fp = parseInt(fb.playlists.length);
        var tabs = $('#tabs');

	settingtabs = true;

	if (tc < fp)
		for(var i = 0; i < fp - tc; ++i)
		{
			$( "<li><a href='#t'></a></li>" )
				.appendTo( "#tabs .ui-tabs-nav" );
			$( "#tabs" ).tabs('refresh');
		}

	if (tc > fp)
		for(var i = 0; i < tc - fp; ++i)
		{
			var tab = $( "#tabs" ).find( ".ui-tabs-nav li:eq("+(i+1)+")" ).remove();
			var panelId = tab.attr( "aria-controls" );
			$( "#" + panelId ).remove();
			$( "#tabs" ).tabs('refresh');
		}


	for (var i = 0; i < fp; ++i)
		if (fb.playlistPlaying == i)
		{
			var c = ''
			if (fb.isPlaying == '1')
				c = '<span class="ui-icon ui-icon-play"';
			else
			if (fb.isPaused == '1')
				c = '<span class="ui-icon ui-icon-pause"';

			$("#tabs a:eq("+i+")").html([c, ' style="position: absolute; margin-top: -2px; margin-left: -5px;"></span><span style="margin-left: 12px;">', fb.playlists[i].name, '</span>'].join(''));
		}
		else
			$("#tabs a:eq("+i+")").html(['<span>', fb.playlists[i].name, '</span>'].join(''));


//	tabs.tabs('select', parseInt(fb.playlistActive));
//	var index = $('#tabs a[href="#simple-tab-2"]').parent().index();
	$("#tabs").tabs("option", "active", parseInt(fb.playlistActive));

	settingtabs = false;
}

function updatealbumartaspect()
{
	if (aa.img.width == 0 || aa.img.height == 0)
		setTimeout('updatealbumartaspect()', refresh_interval);
	else
	{
		if ($('#aa_pane').hasClass("ui-resizeable"))
			$('#aa_pane').resizable('destroy');

		$('#aa_pane').resizable({alsoResize: "#aa_img",
					handles: 'all',
					aspectRatio: aa.img.width/aa.img.height,
					stop: function (event, ui) { 
						save_window_to_cookie('aa_pane', true);
						save_div_position_to_cookie('aa_pane');
					}
  		});
	}
}

function updatealbumart()
{
	if ($('#aa_pane').is(':visible'))
	{
		aa.img = new Image();
		aa.img.src = fb.albumArt;

		$('#aa_img').attr('src', aa.img.src);

		updatealbumartaspect();
	}
}

function updatelibrary()
{
	var a = [];
	for (var i = 0; i < library.query.length; ++i)
		a.push(['<div id="qr" ', (i % 2 == 0? '' : 'class="pl_even"'), '>', stripXmlEntities.perform(library.query[i]), '</div>'].join(''));

	$('#querylist').html(a.join(''));

	$('#querylist div').hover(function() { $(this).addClass('qr_selected'); }, function() { $(this).removeClass('qr_selected'); })
				.off('click').click(function() { retrievelibrarystate('QueryAdvance', ($(this).text())); });

	if (library.queryInfo.length < 1)
		$('#r_btn').html('Query');
	else
		$('#r_btn').text('<');

	for (var i = 0, a = []; i < library.queryInfo.length; ++i)
		a.push(stripXmlEntities.perform(library.queryInfo[i]));

	$('#querypath').html(a.join(' > '));
}

function updateui()
{
	if (fb)
	{
		document.title = stripXmlEntities.perform(fb.helper1) + ' foobar2000';

		if (fb.isPlaying == '1' || fb.isPaused == '1')
			$("#playingtitle").html(fb.helper2);
		else
			$("#playingtitle").html('&nbsp;');

		if (fb.SAC === 'checked')
			$('#progressbar').addClass('ui-state-error');
		else
			$('#progressbar').removeClass('ui-state-error');

		volume_value = fb.volumedb == "1000" ? 0 : 134-Math.round(parseInt(fb.volumedb) / 10.0 / 0.5);
		$("#volume").slider("value", volume_value);

		if (fb.volumedb == 1000)
			$('#mute').addClass('ui-state-error').attr('title', 'Unmute');
		else
			$('#mute').removeClass('ui-state-error').attr('title', 'Mute');

		$("#SAC").prop('checked', fb.SAC === "checked");
		$("#SAQ").prop('checked', fb.SAQ === "checked");
		$("select#PBO").val(fb.playbackOrder);

		if (!drag.dragging)
		{
			mouse.reset();
			drag.reset();
			selection.reset();
		}

		updatealbumart();
		updateplaylist();
		updatetabs();
		updatepreferencesdynamic();
	}

	updatenp();

	tooltip.hide();
}

function command(command, p1, p2)
{
	startwork();

	var params = {};
	if (command) params['cmd'] = command;
	if (p1 || p1 == 0) params['param1'] = p1;
	if (p2 || p2 == 0) params['param2'] = p2;
	params['param3'] = 'NoResponse';

	$.get('/ajquery/', params, function (data) {
		if (!(command == "VolumeDB"))
			retrievestate_schedule(command == "Start" ? 500 : 250);
		else
			finishwork();
	});
}

function retrievestate_schedule(timeout, cmd)
{
	timeout = timeout || 500;
	cmd = cmd || '';

	if (timeoutid)
	{
	  	clearTimeout(timeoutid);
	  	timeoutid = null;
	}

	if (!timeoutid)
	    timeoutid = setTimeout('retrievestate("'+cmd+'")', timeout);
}

function retrievestate(cmd,p1)
{
	startwork();

	if (timeoutid)
	{
	  	clearTimeout(timeoutid);
	  	timeoutid = null;
	}

	cmd = cmd || '';
	p1 = p1 || '';
	cmd = cmd ? ['?cmd=', cmd, '&'].join('') : '?';
	p1 = p1 ? ['&param1=', p1, '&'].join('') : '';

	fb = null;

	$.getJSON(['/ajquery/', cmd, p1, 'param3=js/state.json'].join(''), function(data, status) {
	  	fb = data;	

	  	if (fb.isPlaying == '1' && fb.helper1 == '' || fb.isEnqueueing == '1')
	  	    retrievestate_schedule(refresh_interval);
	  	else
	  	{
			fb.playingItem = parseInt(fb.playingItem);
			fb.prevplayedItem = parseInt(fb.prevplayedItem);
			fb.focusedItem = parseInt(fb.focusedItem);
			fb.playlistItemsCount = parseInt(fb.playlistItemsCount);
			fb.playlistItemsPerPage = parseInt(fb.playlistItemsPerPage);
			fb.playlistPage = parseInt(fb.playlistPage);
			fb.playlistPlaying = parseInt(fb.playlistPlaying);
			fb.playlistActive = parseInt(fb.playlistActive);
			if (fb.playlistPage == 0) fb.playlistPage = 1;

			if (fb.playlists.length == 0)
				fb.playlists = [{ name: '&nbsp;', count: 0 } ];

	  		updateui();
			finishwork();
		}
	});
}

function retrievebrowserstate(p1)
{
	startwork();

	p1 = p1 || '';
	if (p1)
	    p1 = '&param1=' + p1;

	$.getJSON(['/ajquery/?cmd=Browse', p1, '&param3=js/browser.json'].join(''), function(data, status) {
	  	br = data;	

  		updatebrowser();
  		retrievestate();
		finishwork();
	});
}

function retrievelibrarystate(cmd, p1)
{
	startwork();

	cmd = cmd || '';
	p1 = Url.encode(p1);

	$.getJSON(['/ajquery/?cmd=', cmd, '&param1=', p1, '&param3=js/library.json'].join(''), function(data, status) {
	  	library = data;	
  		updatelibrary();
  		command();
	});
}

function positiondialog(dlg,X,Y)
{
	var sw = document.documentElement.clientWidth - 20;
	var dw = dlg.dialog('option', 'width');
	if (X + dw > sw) X = sw - dw;
	if (X < 0) X = 0;
	dlg.dialog('option', 'position', [X, Y]);
}

function searchmedialibrary(v)
{
	command('SearchMediaLibrary', jQuery.trim(v));
}

$(function(){
	$(document).ready(function() {
		$('body').css('overflow','hidden'); // ui dialog bug workaround
		
		$('*').mousemove(function(e){
			mouse.x = e.pageX;
			mouse.y = e.pageY;
		}).mouseup(function() {
			if (drag.dragging)
                        	$("#pl div[id*='i']").mouseup();

			mouse.reset();
			drag.reset();
		}); 
		
		$('#tabs').tabs({
			selected: -1,
			activate: function(event, ui) { 
				if (!settingtabs) retrievestate('SwitchPlaylist', $("#tabs").tabs('option','active'));
			}
		});

		$('#pageslider').slider({
		    change: function(event, ui) { 
		    if (event.originalEvent) 
		    {
			tooltip.hide();
			if (fb && fb.playlistPage != ui.value || !fb)
				retrievestate('P', ui.value);
		    }
		    },
		    slide: function(event,ui) {	
		        tooltip.show( 'Page ' + ui.value + '/' + (Math.ceil(fb.playlistItemsCount*1.0 / fb.playlistItemsPerPage)) , mouse.x+20 , ($(this).offset().top - 15) );
		    },
			range: false,
			min: 1,
			max: 1,            
			value: 1
		});

		// playback preferences dialog
		$('#pbs_dlg').dialog({
			autoOpen: false,
			close: function(event, ui) { 
				save_window_to_cookie($(this).attr('id'), false);
			},
			open: function(event, ui) {
				save_window_to_cookie($(this).attr('id'), true);
				$("#PBO").blur();
			},
			dragStop: function(event, ui) {
				save_dialog_position_to_cookie($(this).attr('id'), ui)
			},
			resizeStop: function(event,ui) { 
				save_dialog_position_to_cookie($(this).attr('id'), ui)
				return true;
			}
		});

		$('#SAC').click(function(e){
			if ($(this).prop('checked'))
				command('SAC', '1');
			else
				command('SAC', '0');
		});

		$('#SAQ').click(function(e){
			if ($(this).prop('checked'))
				command('SAQ', '1');
			else
				command('SAQ', '0');
		});

		$('#PBO').change(function(e){
			command('PlaybackOrder', $(this).prop('selectedIndex'));
		});

		// search dialog
		$('#search_dlg').dialog({
			autoOpen: false,
			width: 300,
			close: function(event, ui) { 
				save_window_to_cookie($(this).attr('id'), false);
			},
			open: function(event, ui) {
				save_window_to_cookie($(this).attr('id'), true);
				$('#searchstr').focus();
			},
			resizable : 'n,e,s,w,nw,sw,ne',
		    	dragStop: function(event, ui) {
				save_dialog_position_to_cookie($(this).attr('id'), ui)
			},
			resizeStop: function(event,ui) { 
				save_dialog_position_to_cookie($(this).attr('id'), ui)
				return true;
			}
		});

		$('#search_dlg_nav1').insertBefore('#search_dlg');
		$('#search_dlg_nav2').insertBefore('#search_dlg');

		$('#searchstr').keypress(function(e) {
			if (e.which == 13)
				searchmedialibrary($('#searchstr').val());
			else
			{
				if (timeoutid2)
				{
				  	clearTimeout(timeoutid2);
				  	timeoutid2 = null;
				}

				if (!timeoutid2)
				    timeoutid2 = setTimeout("searchmedialibrary($('#searchstr').val())", 500);
			}
		});

		$('#r_btn').click(function(e) {
			$(this).blur();
			retrievelibrarystate('QueryRetrace');
		});

		// browse dialog
		$('#browse_dlg').dialog({
			autoOpen: false,
			position: ['left', 'top'],
			open: function(event, ui) {
				retrievebrowserstate(); 
				save_window_to_cookie($(this).attr('id'), true);
			},
			buttons: {
				"Browse to..." : function() { 
					var path = promptf("Browse to:", Url.decode(br.pathcurrent));

					if (path != null)
						retrievebrowserstate(path);
				},
				"Enqueue current" : function() { 
					retrievestate(brparams.cmdenqueue);
				},
				"Enqueue nested" : function() { 
					retrievestate(brparams.cmdenqueuenested);
				},
				"Close": function() { 
					$(this).dialog("close"); 
				}
		    	},
			close: function(event, ui) { 
				save_window_to_cookie($(this).attr('id'), false);
				br_offsets = {};
			},
			dragStop: function(event, ui) {
				save_dialog_position_to_cookie($(this).attr('id'), ui)
			},
			resizeStop: function(event,ui) { 
				save_dialog_position_to_cookie($(this).attr('id'), ui)
				return true;
			}
		});

		$("#browse_dlg_nav").insertBefore("#browse_dlg");

		// error dialog
		$('#error_dlg').dialog({
			autoOpen: false,
			modal: true,
			bgiframe: true,
			width: 500
		});

		//hover states on the static widgets
		$('ul#buttons li:not([id=vol])').hover(
			function() { $(this).addClass('ui-state-hover'); }, 
			function() { $(this).removeClass('ui-state-hover'); }
		);

		// toolbar buttons
		$('.btncmd').click(function(e){
			e.preventDefault();
			command($(this).attr('id'));
		});

		$('#mute').click(function(e){
			command('VolumeMuteToggle');
		});

		$('#pbs_btn').click(function(e){
			set_dialog_state($('#pbs_dlg'), get_values_from_cookie('pbs_dlg'));

			blink_dialog($('#pbs_dlg'));

			$('#pbs_dlg').dialog('open').dialog('moveToTop');
		});

		$('#search_btn').click(function(e){
			set_dialog_state($('#search_dlg'), get_values_from_cookie('search_dlg'));

			blink_dialog($('#search_dlg'));

			$('#search_dlg').dialog('open').dialog('moveToTop');
		});

		$('#browse_btn').click(function(e){
			set_dialog_state($('#browse_dlg'), get_values_from_cookie('browse_dlg'));

			if ($('#browse_dlg').dialog('option', 'height') == 'auto')
			{
				$('#browse_dlg').dialog('option', 'height', 400);
				$('#browse_dlg').dialog('option', 'width', 500);
			}

			blink_dialog($('#browse_dlg'));

			$('#browse_dlg').dialog('open').dialog('moveToTop');
		});

		$('#aa_btn').click(function(e){
			if ($('#aa_pane').is(':hidden'))
			{
				var values = get_values_from_cookie('aa_pane');

				if (values[0] == 'auto' || values[1] == 'auto')
				{
					values[0] = document.documentElement.clientWidth - $('#aa_pane').outerWidth()-5;
					values[1] = 5;
				}

				// stupid workaround for firefox
				if (values[2] != 'auto')
					$('#aa_pane').css( { width: values[2] } );
				if (values[3] != 'auto')
					$('#aa_pane').css( { width: values[3] } );

				$('#aa_pane').css( { position: 'absolute', left: values[0], top: values[1], display: 'inline', visibility: 'visible' } );
				updatealbumart();
				save_window_to_cookie('aa_pane', true);
			}
			else
			{
				$('#aa_pane').hide();
				save_window_to_cookie('aa_pane', false);
			}
		});

		// album art panel
		$('#aa_pane').draggable( {stop: function(event, ui) {
			save_div_position_to_cookie('aa_pane');
		}})
		.dblclick(function(e){
			$('#aa_pane').css({ width: aa.img.width, height: aa.img.height });
			$('#aa_img').css({ width: aa.img.width, height: aa.img.height });
			save_div_position_to_cookie('aa_pane')
		});

		// playback volume slider
		$('#volume').slider({
			range: false,
			min: 0,
			max: 134,
			value: 134,
			slide: function(event, ui) {
				tooltip.show(volume_from_slider(ui), mouse.x+15, $(this).offset().top - 15);
			},
			stop: function(event, ui) {
				volume_from_slider(ui);
				tooltip.hide();
			}
		});

		$('#CreatePlaylist').click(function(e) {
			var name = promptf('Enter new playlist name:', 'New Playlist');

			if (name != null)
				command($(this).attr('id'), name);
		});

		$('#RemovePlaylist').click(function(e) {
			if (!$(this).hasClass('ui-state-disabled'))
				if (confirm("Remove \""+fb.playlists[fb.playlistActive].name+"\" playlist?"))
					command($(this).attr('id'), fb.playlistActive);
		});

		$('#RenamePlaylist').click(function(e) {
			if (!$(this).hasClass('ui-state-disabled'))
			{
				var new_name = promptf("Enter new name:", fb.playlists[fb.playlistActive].name);

				if (new_name != null)
					command('RenamePlaylist', new_name, fb.playlistActive);
			}
		});

		$('#EmptyPlaylist').click(function(e) {
			if (!$(this).hasClass('ui-state-disabled'))
				if (confirm("Empty playlist?"))
					command($(this).attr('id'));
		});

		$('#RefreshPlaylist').click(function(e) {
			command();
		});

		$('#FlushQueue,#QueueRandomItems').click(function(e) {
			command($(this).attr('id'));
		});

		$('#Undo,#Redo').click(function() {
			if (!$(this).hasClass('ui-state-disabled'))
				command($(this).attr('id'));
		});

		$('#FocusOnPlaying').click(function() {
			if (!$(this).hasClass('ui-state-disabled'))
				command('FocusOnPlaying');
		});

		$('#pageslider').dblclick(function(e) {
			$('#FocusOnPlaying').click();
		});

		$('#QueueItems,#DequeueItems,#Del,#SetFocus').click(function() { 
			if (!$(this).hasClass('ui-state-disabled'))
			{
				var items = selection.tostr();
				command($(this).attr('id'), items);
			}
		});

		// Playlist buttons
		$('#prevpage_btn').click(function(e){
			if (!$(this).hasClass('ui-state-disabled') && fb) {
				e.preventDefault();
				retrievestate('P', fb.playlistPage - 1);
			}
		});

		$('#nextpage_btn').click(function(e){
			if (!$(this).hasClass('ui-state-disabled')) {
				e.preventDefault();
				retrievestate('P', fb.playlistPage + 1);
			}
		});

		$('#searchstr,#playlist_name').blur(function() { input_has_focus = false; }).focus(function() { input_has_focus = true; });

		$('#progressbar').progressbar({
			value: 0
		})                                                             
		.resizable({ alsoResize: '#tabs', handles: 'e', resize: function(event, ui) { save_playlist_size(); } })
		.hover(
			function(event) { },
			function(event) { tooltip.hide(); })
		.mousemove(
			function(event) {
				if (fb && fb.isPlaying == "1")
				{
					var t = Math.round((event.pageX-$('#progressbar').offset().left)*fb.itemPlayingLen / ($('#progressbar').width()));
				        tooltip.show(format_time(t), event.pageX+15, $('#progressbar').offset().top-10);
				}
		})
		.click(	function(event) {
				if (fb && fb.isPlaying == "1")
					command('SeekSecond', (Math.round((mouse.x-$('#progressbar').offset().left)*fb.itemPlayingLen / $('#progressbar').width())));
			});

		$(window).bind("blur", function() {
			keypressed = { };
		});

		$(document).ajaxError(function(event, XMLHttpRequest, settings, thrownError){
			if(event.type == 'ajaxError')
			{
				var rep = '';
				rep += 'settings.url: ' + settings.url + '\n\n';
				rep += 'XMLHttpRequest.responseText: \n' + XMLHttpRequest.responseText + '\n\n';
				rep += 'XMLHttpRequest.statusText: ' + XMLHttpRequest.statusText;
				$('#dbg').html(rep);
				$('#error_dlg').dialog('open');
			}
		});

		document.onkeydown = function(evt) { keydown(evt?evt.keyCode:event.keyCode); }
		document.onkeyup = function(evt) { keyup(evt?evt.keyCode:event.keyCode); }

		retrievestate();
		reopen_windows();
		restore_playlist_size();
	});
});