function create_xml_http_request()
{
	var request = false;
	
	if (window.XMLHttpRequest) 
	{ // most browsers
		request = new XMLHttpRequest();
//		if (request.overrideMimeType) {
//			request.overrideMimeType('text/xml');
//		}
	}
	else if (window.ActiveXObject) 
	{ //ie
		try 
		{
			request = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch(e) 
		{
			try { request = new ActiveXObject("Microsoft.XMLHTTP"); }
			catch(e) { }
		}
	}
	
	return request;	
}


function fetch_xml(url, callback_function,error_function) 
{
	var request = create_xml_http_request();
	if (!request) 
	{ 
		// Browser doesn't support XMLHttpRequest
		error_function("Browser doesn't support XMLHttpRequest");
		return false;
	}
	
	request.onreadystatechange = function() 
	{
		if (request.readyState == 4) 
		{
			if (request.status == 200) 
			{
				//overrideMimeType didn't work in Konqueror,
				//so we'll have to parse the response into XML
				//object ourselfs. responseXML won't work.
				var xmlstring = request.responseText;
				var xmldoc;
				if (window.DOMParser) 
				{
					xmldoc = (new DOMParser()).parseFromString(xmlstring, "text/xml");
				}
				else if (window.ActiveXObject) 
				{ //ie
					xmldoc = new ActiveXObject("Microsoft.XMLDOM");
					xmldoc.async = false;
					xmldoc.loadXML(xmlstring);
				}

				callback_function(xmldoc);
			}
			else 
			{
				// could not fetch
				error_function("Could not fetch " + url);
			}
		}
	}
	
	request.open('GET', url, true);
	request.send(null);
}

function fetch_xml_post(url,post_data,callback_function,error_function)
{
	var request = create_xml_http_request();
	if (!request) 
	{ 
		// Browser doesn't support XMLHttpRequest
		error_function("Browser doesn't support XMLHttpRequest");
		return false;
	}
	
	request.onreadystatechange = function() 
	{
		if (request.readyState == 4) 
		{
			if (request.status == 200) 
			{
				//overrideMimeType didn't work in Konqueror,
				//so we'll have to parse the response into XML
				//object ourselfs. responseXML won't work.
				var xmlstring = request.responseText;
				var xmldoc;
				if (window.DOMParser) 
				{
					xmldoc = (new DOMParser()).parseFromString(xmlstring, "text/xml");
				}
				else if (window.ActiveXObject) 
				{ //ie
					xmldoc = new ActiveXObject("Microsoft.XMLDOM");
					xmldoc.async = false;
					xmldoc.loadXML(xmlstring);
				}

				callback_function(xmldoc);
			}
			else 
			{
				// could not fetch
				error_function("Could not fetch " + url);
			}
		}
	}
	
	request.open('POST', url, true);
	request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	request.setRequestHeader("Content-length", post_data.length);
	request.setRequestHeader("Connection", "close");
	request.send(post_data);
}