function redirect_to_login(msg)
{
	window.location = "/login.html";
}

function show_div(div)
{
	divs = new Array('content','torrent_details','torrent_load','settings');
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

function clear_error()
{
	var p = document.getElementById('error_text');
	p.innerHTML = "";
	p.style.display = "none";
}

//show error div with this error message in it
function show_error(msg)
{
	var p = document.getElementById('error_text');
	p.innerHTML = "Error: " + msg;
	p.style.display = "";
}

function update_status_bar()
{
	fetch_xml("/data/global.xml",update_status_bar_table,show_error);
}

function redirect_to_login()
{
	window.location = "/login.html";
}

function update_status_bar_table(xmldoc) 
{
	var d = document.getElementById('dht');
	var dht = get_tag_data(xmldoc, 'dht') == "1" ? "On" : "Off";
	d.innerHTML = "DHT: <strong>" + dht + "</strong>";

	d = document.getElementById('encryption');
	var encryption = get_tag_data(xmldoc, 'encryption') == "1" ? "On" : "Off";
	d.innerHTML = "Encryption: <strong>" + encryption + "</strong>";	
	
	d = document.getElementById('speed');
	var down = get_tag_data(xmldoc, 'speed_down');
	var up   = get_tag_data(xmldoc, 'speed_up');
	d.innerHTML = "Speed down: <strong>" + down + "</strong> / up: <strong>" + up + "</strong>";
	
	d = document.getElementById('transferred');
	down = get_tag_data(xmldoc, 'transferred_down');
	up   = get_tag_data(xmldoc, 'transferred_up');
	d.innerHTML = "Transferred down: <strong>" + down + "</strong> / up: <strong>" + up + "</strong>";
}

function get_tag_data(element, tag) 
{
	var text_node;
	try 
	{
		text_node = element.getElementsByTagName(tag)[0].firstChild.data;
	}
	catch (e) 
	{
		text_node = "";
	}
	return text_node;
}
