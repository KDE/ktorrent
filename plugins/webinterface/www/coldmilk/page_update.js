  /***************************************************************************
 *   Copyright (C) 2007 by Dagur Valberg Johannsson                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

var details_of_torrent = null; //id of torrent which details are displayed
var interval_timer = null;

function redirect_to_login()
{
	window.location = "/login.html";
}

function automatic_refresh()
{
	var element = document.getElementById('webgui_automatic_refresh');
	return element && element.checked;
}

function automatic_refresh_changed(value)
{
	if (value)
	{
		if (!interval_timer)
			interval_timer = window.setInterval(update_all, 5000, false);
	}
	else
	{
		if (interval_timer)
		{
			// stop the interval timer
			clearInterval(interval_timer);
			interval_timer = null;
		}
	}
}

function update_all(force) 
{
	fetch_xml("data/global.xml",update_status_bar_and_title,redirect_to_login);
	if (div_visible('torrent_list'))
		fetch_xml("data/torrents.xml",update_torrent_table,redirect_to_login);
	
	if (automatic_refresh() || force)
	{
		if (!interval_timer)
			interval_timer = window.setInterval(update_all, 5000, false);
	}
	else if (interval_timer)
	{
		// stop the interval timer
		clearInterval(interval_timer);
		interval_timer = null;
	}
}

function update_status_bar_and_title(xmldoc)
{
	update_title(xmldoc);
	update_status_bar(xmldoc);
}

function update_title(xmldoc) 
{
	var down = _get_text(xmldoc, 'speed_down').data;
	var up   = _get_text(xmldoc, 'speed_up').data;
	var new_title = "(D: " + down + ") (U: " + up + ") - ktorrent web interface";
	document.title = new_title;
}

function update_status_bar(xmldoc) 
{
	var newtable = document.createElement('table');
	newtable.setAttribute('id', 'status_bar_table');

	
	//dht and encryption
	{
		var row = newtable.insertRow(0);
		var cell = row.insertCell(0);
		var dht = _get_text(xmldoc, 'dht').data;
		var encryption = _get_text(xmldoc, 'encryption').data;
		cell.appendChild(document.createTextNode("DHT : " + ((dht == 1) ? "On" : "Off")));
		cell = row.insertCell(1);
		cell.appendChild(document.createTextNode("Encryption : " + ((encryption == 1) ? "On" : "Off")));
	}	
	//speed down/up
	{
		var row = newtable.insertRow(1);
		var cell = row.insertCell(0);
		cell.appendChild(
			document.createTextNode("Speed"));
		
		cell = row.insertCell(1);
		var down = _get_text(xmldoc, 'speed_down').data;
		var up   = _get_text(xmldoc, 'speed_up').data;
		cell.appendChild(
			document.createTextNode("down: " + down + " / up: " + up));
	}
	//transferred
	{
		var row = newtable.insertRow(2);
		var cell = row.insertCell(0);
		cell.appendChild(
			document.createTextNode("Transferred"));
		
		cell = row.insertCell(1);
		var down = _get_text(xmldoc, 'transferred_down').data;
		var up   = _get_text(xmldoc, 'transferred_up').data;
		cell.appendChild(
			document.createTextNode("down: " + down + " / up: " + up));
	}
	var oldtable = document.getElementById('status_bar_table');
	oldtable.parentNode.replaceChild(newtable, oldtable);
}

function update_torrent_table(xmldoc) 
{
	var newtable = document.createElement('table');
	newtable.setAttribute('id', 'torrent_list_table');
	newtable.className='list_table';

	var torrents = xmldoc.getElementsByTagName('torrent');
	var i = 0;
	while (torrents[i]) {
		_torrent_table_row(torrents[i], newtable, i);
		i++;
	}
	_torrent_table_header(newtable.insertRow(0));

	var oldtable = document.getElementById('torrent_list_table');
	oldtable.parentNode.replaceChild(newtable, oldtable);
}

function _torrent_table_row(torrent, table, i) 
{
	var row = table.insertRow(i);
	var row_color = (i % 2) ?
		"#ffffff" : "#dce4f9";
	row.setAttribute("style", "background-color : " + row_color);

	//actions
	{
		var cell = row.insertCell(0);
		var can_start = (_get_text(torrent, 'running').data == "1") ? 0 : 1; //if torrent is running we can't start it
		var can_stop = (can_start==1) ? 0 : 1; //opposite of can_start
		var start_button  = _create_action_button('Start', 'icon?name=kt-start&size=1', (can_start==1) ? 'start='+i : '');
		var stop_button   = _create_action_button('Stop', 'icon?name=kt-stop&size=1', (can_stop==1) ? 'stop='+i : '');
		var remove_button = _create_action_button('Remove', 'icon?name=kt-remove&size=1', 'remove='+i);
		remove_button.setAttribute("onclick", "return validate('remove_torrent')");
		
		cell.appendChild(start_button);
		cell.appendChild(stop_button);
		cell.appendChild(remove_button);
	}

	//file
	{
		var cell = row.insertCell(1);
		if (_get_text(torrent, 'num_files').data == 0)
		{
			cell.appendChild(_get_text(torrent, 'name'));
		}
		else
		{
			var file = document.createElement('a');
			file.setAttribute('href', '#');
			file.appendChild(_get_text(torrent, 'name'));
			file.onclick = function()
			{
				show('torrents_details');
				fetch_xml("data/torrent/files.xml?torrent="+i, get_torrents_details,redirect_to_login);
				details_of_torrent = i;
			};
			cell.appendChild(file);
		}
	}

	//status
	{
		var cell = row.insertCell(2);
		cell.appendChild(_get_text(torrent, 'status'));
	}

	//speed
	{
		var cell = row.insertCell(3);
		
		cell.appendChild(_get_text(torrent, 'download_rate'));
		cell.appendChild(document.createElement('br'));
		cell.appendChild(_get_text(torrent, 'upload_rate'));
	}

	//size
	{
		var cell = row.insertCell(4);
		cell.appendChild(_get_text(torrent, 'total_bytes_to_download'));
	}

	//peers
	{
		var cell = row.insertCell(5);
		cell.appendChild(_get_text(torrent, 'num_peers'));
	}

	//transferred
	{
		var cell = row.insertCell(6);
		
		cell.appendChild(_get_text(torrent, 'bytes_downloaded'));
		cell.appendChild(document.createElement('br'));
		cell.appendChild(_get_text(torrent, 'bytes_uploaded'));	
	}

	//done
	{
		var cell = row.insertCell(7);
		cell.setAttribute("style", "padding-right : 2px;");

		var percent_done = _get_text(torrent, 'percentage').data;

		var bar = document.createElement('div');
		bar.setAttribute("class", "percent_bar");
		bar.setAttribute("style", "width : " + percent_done + "%;");
		cell.appendChild(bar);

		var bar_text = document.createElement('div');
		bar_text.appendChild(document.createTextNode(percent_done + "%"));

		bar.appendChild(bar_text);
	}
}

//function called after changing file priority to refresh list of files (and priorities)
function just_refresh_details(xmldoc) 
{
	if (details_of_torrent!=null)
		fetch_xml("data/torrent/files.xml?torrent="+details_of_torrent,get_torrents_details,redirect_to_login);
}

function get_torrents_details(xmldoc) 
{
	var newtable = document.createElement('table');
	newtable.setAttribute('id', 'torrents_details_files');
	newtable.className='list_table';

	var id = details_of_torrent;
	var files = xmldoc.getElementsByTagName('file');
	for(var i=0; i<files.length; i++)
	{
		var row = newtable.insertRow(i);
		row.style.backgroundColor=(i % 2) ? '#ffffff' : '#dce4f9';
		var cell = row.insertCell(-1);

		var file_status = _get_text(files[i], 'priority').data;
		var command; //we call ?file_xx - this call is detected by server and priority is being changed

		command = (file_status==50)?'':"javascript:do_action('file_hp=" + id + "-" + i + "');";
		var high_prior = _create_file_action_button('high_priority.png', 'High Priority', command);
		cell.appendChild(high_prior);

		command = (file_status==40)?'':"javascript:do_action('file_np=" + id + "-" + i + "');";
		var normal_prior = _create_file_action_button('normal_priority.png', 'Normal Priority', command);
		cell.appendChild(normal_prior);

		command = (file_status==30)?'':"javascript:do_action('file_lp=" + id + "-" + i + "');";
		var low_prior = _create_file_action_button('low_priority.png', 'Low Priority', command);
		cell.appendChild(low_prior);

		command = (file_status==20 || file_status==10)?'':"javascript:do_action('file_stop=" + id + "-" + i + "');";
		var dnd = _create_file_action_button('only_seed.png', 'Stop downloading (Only Seed Priority)', command);
		cell.appendChild(dnd);

		var cell = row.insertCell(-1);
		cell.appendChild(_get_text(files[i], 'path'));
		var cell = row.insertCell(-1);
		cell.appendChild(_get_text(files[i], 'size'));
		var cell = row.insertCell(-1);

		if (_get_text(files[i], 'percentage').data!='')
			cell.appendChild(_get_text(files[i], 'percentage'));
		else
			cell.appendChild(document.createTextNode("0"));
		cell.appendChild(document.createTextNode("%"));
		var cell = row.insertCell(-1);

		cell.appendChild(document.createTextNode(_get_file_status_name(file_status)));
	}

	_torrents_details_header(newtable.insertRow(0));

	/*var torrents = xmldoc.getElementsByTagName('torrent');
	var i = 0;
	while (torrents[i]) {
		_torrent_table_row(torrents[i], newtable, i);
		i++;
	}
	_torrent_table_header(newtable.insertRow(0));*/

	var oldtable = document.getElementById('torrents_details_files');
	oldtable.parentNode.replaceChild(newtable, oldtable);
}

function _create_action_button(button_name, image_src, command) 
{
	var image = document.createElement("img");
	image.setAttribute("src", image_src);
	image.setAttribute("alt", button_name);
	image.setAttribute("title", button_name);

	if (command != '')
	{
		var a = document.createElement("a");
		a.setAttribute("href", "javascript:do_action('" + command + "'); update_all();");
		a.appendChild(image);
		return a;
	}
	else
		return image;
}

function _create_file_action_button(img_src, img_alt, command) 
{
	var image = document.createElement("img");
	image.setAttribute("src", img_src);
	image.setAttribute("alt", img_alt);
	image.setAttribute("title", img_alt);
	if (command != '')
	{
		var a = document.createElement("a");
		a.setAttribute("href", command  + " just_refresh_details();");
		a.appendChild(image);
		return a;
	}
	else
	return image;
}

// gets element with given tag and crates text node from it
function _get_text(element, tag) 
{
	var text_node;
	try 
	{
		text_node = document.createTextNode(
			element.getElementsByTagName(tag)[0].firstChild.data);
	}
	catch (e) 
	{
		text_node = document.createTextNode('');
	}
	return text_node;
}

function _get_text_from_attribute(element, tag, attribute) 
{
	var text_node;
	try 
	{
		text_node = document.createTextNode(
			element.getElementsByTagName(tag)[0].getAttribute(attribute));
	}
	catch (e) 
	{
		text_node = document.createTextNode('');
	}
	return text_node;
}

function _get_file_status_name(status_id)
{
	if (status_id==60) return 'PREVIEW_PRIORITY';
	else if (status_id==50) return 'Download First';
	else if (status_id==40) return 'Download Normally';
	else if (status_id==30) return 'Download Last';
	else if (status_id==20) return 'Only Seed';
	else if (status_id==10) return 'Do Not Download';
	else return 'Not supported file status';
}

function _torrents_details_header(row) 
{
	headers = new Array("Actions", "File", "Size", "Perc done", "Status");
	for (var i in headers) 
	{
		var header =  document.createElement("th");
		header.appendChild(document.createTextNode(headers[i]));
		row.appendChild(header);
	}
	return row;
}

function _torrent_table_header(row) 
{
	headers = new Array(
		"Actions", "File", "Status", 
		"Speed", "Size", "Peers", 
		"Transferred", "% done"
	);

	for (var i in headers) 
	{
		var header =  document.createElement("th");
		header.appendChild(
			document.createTextNode(headers[i]));
		row.appendChild(header);
	}
	return row;
}
