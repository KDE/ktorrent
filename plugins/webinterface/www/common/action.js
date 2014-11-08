// do an action 
function do_action(cmd)
{
	fetch_xml("/action?" + cmd,action_ok,action_error);
}

function action_ok(xmldoc)
{
	if (xmldoc.getElementsByTagName("result")[0].data == "Failed")
		show_error("Error: action failed !");
}

function action_error(msg)
{
	show_error(msg);
}

function shutdown()
{
	fetch_xml("/action?shutdown=true",action_ok,action_error);
	window.location = "/login.html";
}

function load_url()
{
	var url = document.getElementById("torrent_url");
	var eurl = escape(url.value);
	fetch_xml("/action?load_torrent=" + escape(eurl),action_ok,action_error);
	url.value = "";
}