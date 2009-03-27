function update_settings(err_function)
{
	fetch_xml("data/settings.xml",update_settings_div,err_function);
}

function update_settings_div(xmldoc)
{
	var node = document.getElementById("settings_form");
	var inputs = node.getElementsByTagName("input");

	for (i = 0;i < inputs.length;i++)
	{
		var input = inputs[i];
		// try to find the element in the settings
		var el = get_settings_element(xmldoc,input.name);
		if (el)
		{
			// set input element's value if we found it
			if (input.type == "text")
				input.value = el.textContent;
			else if (input.type == "checkbox")
				input.checked = el.textContent == "true";
		}
	}
}

function get_settings_element(settings,name)
{
	var el = settings.getElementsByTagName(name);
	if (el && el.length >= 1)
	{
		return el[0];
	}
	return false;
}

function submit_settings(err_function)
{
	var node = document.getElementById("settings_form");
	var inputs = node.getElementsByTagName("input");
	var post_data = "";

	for (i = 0;i < inputs.length;i++)
	{
		var input = inputs[i];
		if (i != 0)
			post_data += "&";
		
		if (input.type == "checkbox")
			post_data += input.name + "=" + (input.checked ? "1" : "0");	
		else
			post_data += input.name + "=" + encodeURI(input.value);
	}
	
	fetch_xml_post("data/settings.xml",post_data,update_settings_div,err_function);
}