function refresh()
{
	clear_error(); 
	update_torrents();
}

function show_div(div)
{
	var divs = new Array('content','torrent_details','torrent_load','settings');
	for (var d in divs) 
	{
		var element = document.getElementById(divs[d]);
		if (element)
		{
			if (divs[d] == div)
				element.style.display = "";
			else
				element.style.display = "none";
		}
	}
}

function redirect_to_login()
{
	window.location = "login.html";
}

function clear_error()
{
	var p = document.getElementById('error');
	p.style.display = "none";
}

//show error div with this error message in it
function show_error(msg)
{
	var p = document.getElementById('error_text');
	p.innerHTML = "Error: " + msg;
	
	var d = document.getElementById('error');
	d.style.display = "block";
}

function update_torrents()
{
	clear_error();
	fetch_xml("data/torrents.xml",update_torrent_list,show_error);
}

function update_torrent_list(xmldoc)
{
	var newtable = document.createElement('table');
	newtable.setAttribute('id', 'torrent_list_table');
	
	var torrents = xmldoc.getElementsByTagName('torrent');
	var i = 0;
	while (torrents[i]) 
	{
		var row = newtable.insertRow(i);
		var cell = row.insertCell(0);
		cell.appendChild(torrent_item_div(torrents[i],i));
		i++;
	}
	
	var oldtable = document.getElementById('torrent_list_table');
	oldtable.parentNode.replaceChild(newtable, oldtable);
}

function torrent_item_div(torrent,i) 
{
	var div = document.createElement('div');
	div.setAttribute('class','torrent_list_item');
	
	var actions = document.createElement('div');
	actions.setAttribute('class','line_item');
	actions.innerHTML = 
		"<a href=\"javascript:do_action('start=" + i + "')\"><img src=\"icon?name=kt-start&size=1\" /></a>" +
		"<a href=\"javascript:do_action('stop=" + i + "')\"><img src=\"icon?name=kt-stop&size=1\" /></a>" +
		"<a href=\"javascript:do_action('remove=" + i + "')\"><img src=\"icon?name=kt-remove&size=1\" /></a>";
	var title = document.createElement('div');
	title.setAttribute('class','torrent_name');
	if (get_text_data(torrent,'num_files') == '0')
	{
		title.appendChild(get_text(torrent,'name',false));
	}
	else
	{
		var a = document.createElement('a');
		a.setAttribute('href',"javascript:show_div('torrent_details'); update_torrent_details(" + i + ");");
		a.appendChild(get_text(torrent,'name',false));
		title.appendChild(a);
	}
	
	var header = document.createElement('div');
	header.setAttribute('class','torrent_list_item_header');
	header.appendChild(actions);
	header.appendChild(title);
	div.appendChild(header);
	
	var line = document.createElement('div');
	line.setAttribute('class','torrent_list_item_line');
	var item = document.createElement('div');
	item.setAttribute('class','line_item');
	item.innerHTML = 'Status: <strong>' + get_text_data(torrent,'status') + '</strong>';
	line.appendChild(item);
	div.appendChild(line);
	
	line = document.createElement('div');
	line.setAttribute('class','torrent_list_item_line');
	item = document.createElement('div');
	item.setAttribute('class','line_item');
	item.innerHTML = 
			'Downloaded: <strong>' + get_text_data(torrent,'bytes_downloaded') + ' / ' 
			+ get_text_data(torrent,'total_bytes_to_download') + '</strong>';
	line.appendChild(item);
	item = document.createElement('div');
	item.setAttribute('class','line_item');
	item.innerHTML = 'Uploaded: <strong>' + get_text_data(torrent,'bytes_uploaded') + '</strong>';
	line.appendChild(item);
	div.appendChild(line);
	
	line = document.createElement('div');
	line.setAttribute('class','torrent_list_item_line');
	item = document.createElement('div');
	item.setAttribute('class','line_item');
	item.innerHTML = 
			'Speed: <strong>' + get_text_data(torrent,'download_rate') + '</strong> down, <strong>' 
			+ get_text_data(torrent,'upload_rate') + '</strong> up';
	line.appendChild(item);
	item = document.createElement('div');
	item.setAttribute('class','line_item');
	item.innerHTML = 'Peers: <strong>' + get_text_data(torrent,'num_peers') + '</strong>';
	line.appendChild(item);
	div.appendChild(line);
	return div;
}

function get_text(element,tag,strong) 
{
	var text_node;
	try 
	{
		text_node = document.createTextNode(element.getElementsByTagName(tag)[0].firstChild.data);
		if (strong)
		{
			var s = document.createElement('strong');
			s.appendChild(text_node);
			return s;
		}
	}
	catch (e) 
	{
		text_node = document.createTextNode('');
	}
	return text_node;
}

function get_text_data(element,tag)
{
	var text_node;
	try
	{
		text_node = element.getElementsByTagName(tag)[0].firstChild.data;
	}
	catch (e)
	{
		text_node = '';
	}
	return text_node;
}
