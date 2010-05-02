
function update_torrents()
{
	fetch_xml("data/torrents.xml",update_data_table,show_error)
}

function update_data_table(xmldoc)
{
	// Make sure errors are cleared
	clear_error();
	
	var newtable = document.createElement('table');
	newtable.setAttribute('id', 'torrent_list_table');
	newtable.className='list_table';
	
	var torrents = xmldoc.getElementsByTagName('torrent');
	var i = 0;
	while (torrents[i]) 
	{
		torrent_table_row(torrents[i], newtable, i);
		i++;
	}
	
	torrent_table_header(newtable.insertRow(0));

	var oldtable = document.getElementById('torrent_list_table');
	oldtable.parentNode.replaceChild(newtable, oldtable);
}

function torrent_table_row(torrent, table, i) 
{
	var row = table.insertRow(i);
	var row_color = (i % 2) ? "#ffffff" : "#dce4f9";
	row.setAttribute("style", "background-color : " + row_color);

	//actions
	{
		var cell = row.insertCell(0);
		var can_start = (get_text(torrent, 'running').data == "1") ? 0 : 1; //if torrent is running we can't start it
		var can_stop = (can_start==1) ? 0 : 1; //opposite of can_start
		var start_button  = create_action_button('Start', 'icon?name=kt-start&size=1', (can_start==1) ? 'start='+i : '');
		var stop_button   = create_action_button('Stop', 'icon?name=kt-stop&size=1', (can_stop==1) ? 'stop='+i : '');
		var remove_button = create_action_button('Remove', 'icon?name=kt-remove&size=1', 'remove='+i);
		remove_button.setAttribute("onclick", "return validate('remove_torrent')");
		
		cell.appendChild(start_button);
		cell.appendChild(stop_button);
		cell.appendChild(remove_button);
		cell.setAttribute("align","center");
	}

	//file
	{
		var cell = row.insertCell(1);
		var tor = get_text(torrent, 'name');
		if (torrent.getElementsByTagName('num_files')[0].firstChild.data == '0')
		{
			cell.appendChild(tor);
		}
		else
		{
			var a = document.createElement("a");
			a.setAttribute('href','javascript:show_torrent_details(' + i + ')');
			a.appendChild(tor);
			cell.appendChild(a);
		}
	}

	//status
	{
		var cell = row.insertCell(2);
		cell.appendChild(get_text(torrent, 'status'));
	}
	
	// transferred
	{
		var cell = row.insertCell(3);
		var up = get_text(torrent, 'bytes_uploaded');
		var down = get_text(torrent, 'bytes_downloaded');
		up.appendData(" up");
		down.appendData(" down");
		cell.appendChild(down);
		cell.appendChild(document.createElement('br'));
		cell.appendChild(up);
	}
	
	//size
	{
		var cell = row.insertCell(4);
		cell.appendChild(get_text(torrent, 'total_bytes_to_download'));
	}
	
	// speed
	{
		var cell = row.insertCell(5);
		var down = get_text(torrent, 'download_rate');
		var up = get_text(torrent, 'upload_rate');
		up.appendData(" up");
		down.appendData(" down");
		cell.appendChild(down);
		cell.appendChild(document.createElement('br'));
		cell.appendChild(up);
	}
	
	//peers
	{
		var cell = row.insertCell(6);
		var seeders = get_text(torrent, 'seeders');
		var leechers = get_text(torrent, 'leechers');
		seeders.appendData(" seeders");
		leechers.appendData(" leechers");
		cell.appendChild(seeders);
		cell.appendChild(document.createElement('br'));
		cell.appendChild(leechers);
	}
	
	// percentage
	{
		var cell = row.insertCell(7);
		var node = get_text(torrent, 'percentage');
		node.appendData(' %');
		cell.appendChild(node);
	}
	
}

function torrent_table_header(row) 
{
	headers = new Array(
 "Actions","File","Status","Transferred","Size",
 "Speed","Peers","Complete"
	);

	for (var i in headers) 
	{
		var header =  document.createElement("th");
		header.appendChild(document.createTextNode(headers[i]));
		row.appendChild(header);
	}
	return row;
}

// gets element with given tag and crates text node from it
function get_text(element, tag) 
{
	var text_node;
	try 
	{
		text_node = document.createTextNode(element.getElementsByTagName(tag)[0].firstChild.data);
	}
	catch (e) 
	{
		text_node = document.createTextNode('');
	}
	return text_node;
}

function create_action_button(button_name, image_src, command) 
{
	var image = document.createElement("img");
	image.setAttribute("src", image_src);
	image.setAttribute("alt", button_name);
	image.setAttribute("title", button_name);

	if (command != '')
	{
		var a = document.createElement("a");
		a.setAttribute("href", "javascript:do_action(\"" + command + "\"); update_torrents();");
		a.appendChild(image);
		return a;
	}
	else
		return image;
}

function validate(action)
{
	var msg;
	if (action == "remove_torrent") {
		msg = "Are you sure that you want remove this torrent?";
	}
	else if (action == "quit_program") {
		msg = "Are you sure you want to quit ktorrent?";
	}
	else {
		msg = "Do it?";
	}
	return confirm(msg);
}