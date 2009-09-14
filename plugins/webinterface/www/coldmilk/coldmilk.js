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
};

function show(id)
{
	var items = new Array();
	items[0] = "torrent_list";
	items[1] = "torrents_details";
	items[2] = "preferences";
	items[3] = "torrent_add";
	items[4] = "action";
	
	hide_divs(items);

	// Show selected
	var item_show = document.getElementById(id);
	item_show.style.display = "";
	if (id == 'preferences')
		update_settings(redirect_to_login);
};

function hide_divs(items) 
{
	for (var i in items) 
	{
		var item_hide = document.getElementById(items[i]);
		if (item_hide != null && !item_hide.style.display) 
		{ // not means it's showing..
			item_hide.style.display = "none";
		}
	}
	return true;
}
	
function div_visible(id)
{
	var item = document.getElementById(id);
	return item.style.display != "none";
}
