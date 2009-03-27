var current_torrent = 0;

function show_torrent_details(tor)
{
	clear_torrent_details();
	update_torrent_details(tor);
	show_div("torrent_details");
}

function clear_torrent_details()
{
	var newtable = document.createElement('table');
	newtable.setAttribute('id', 'torrent_details_table');
	newtable.className='list_table';
	
	file_header(newtable.insertRow(0));
	
	var oldtable = document.getElementById('torrent_details_table');
	oldtable.parentNode.replaceChild(newtable, oldtable);
}

function update_torrent_details(tor)
{
	current_torrent = tor;
	fetch_xml("data/torrent/files.xml?torrent=" + tor,update_torrent_details_table,show_error);
}

function update_torrent_details_table(xmldoc)
{
	// Make sure errors are cleared
	clear_error();
	
	var newtable = document.createElement('table');
	newtable.setAttribute('id', 'torrent_details_table');
	newtable.className='list_table';
	
	var files = xmldoc.getElementsByTagName('file');
	var i = 0;
	while (files[i]) 
	{
		file_row(files[i], newtable, i);
		i++;
	}
	
	file_header(newtable.insertRow(0));

	var oldtable = document.getElementById('torrent_details_table');
	oldtable.parentNode.replaceChild(newtable, oldtable);
}

function file_row(element,table,i)
{
	var row = table.insertRow(i);
	var actions = row.insertCell(0);
	var file = row.insertCell(1);
	var priority = row.insertCell(2);
	var size = row.insertCell(3);
	var perc = row.insertCell(4);
	
	row.setAttribute('class',(i % 2 == 0) ? 'even' : 'odd');
	
	var file_status = element.getElementsByTagName('priority')[0].firstChild.data;
	var command = '';
	command = (file_status==20 || file_status==10) ? '' : "file_stop=" + current_torrent + "-" +i;
	actions.appendChild(create_priority_button("Only Seed","only_seed.png",command));
	
	command = (file_status==30) ? '' : "file_lp=" + current_torrent + "-" +i;
	actions.appendChild(create_priority_button("Low Priority","low_priority.png",command));
	
	command = (file_status==40) ? '' : "file_np=" + current_torrent + "-" +i;
	actions.appendChild(create_priority_button("Normal Priority","normal_priority.png",command));
	
	command = (file_status==50) ? '' : "file_hp=" + current_torrent + "-" +i;
	actions.appendChild(create_priority_button("High Priority","high_priority.png",command));
	
	actions.setAttribute("align","center");
	file.appendChild(get_text(element,"path"));
	size.appendChild(get_text(element,"size"));
	priority.appendChild(priority_node(element));
	var el = get_text(element,"percentage");
	el.appendData(" %");
	perc.appendChild(el);
}

function file_header(row)
{
	headers = new Array("Actions","File","Priority","Size","Complete");
	for (var i in headers) 
	{
		var header =  document.createElement("th");
		header.appendChild(document.createTextNode(headers[i]));
		row.appendChild(header);
	}
	return row;
}


function priority_node(element)
{
	var prio = element.getElementsByTagName('priority')[0].firstChild.data;
	switch (prio)
	{
	case '50': return document.createTextNode('Download First');
	case '40': return document.createTextNode('Download Normally');
	case '30': return document.createTextNode('Download Last');
	case '20': return document.createTextNode('Only Seed');
	case '10': return document.createTextNode('Do Not Download');
	default: return document.createTextNode('Unknown');
	}
}

function create_priority_button(button_name, image_src, command) 
{
	var image = document.createElement("img");
	image.setAttribute("src", image_src);
	image.setAttribute("alt", button_name);
	image.setAttribute("title", button_name);

	if (command != '')
	{
		var a = document.createElement("a");
		a.setAttribute("href", "javascript:do_action(\"" + command + "\"); update_torrent_details(current_torrent);");
		a.appendChild(image);
		return a;
	}
	else
		return image;
}
