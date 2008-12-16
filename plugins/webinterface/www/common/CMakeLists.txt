set(common_data sha1.js
	favicon.ico 
	ktorrentwebinterfacelogo.png 
	start.png 
	stop.png 
	remove.png 
	normal_priority.png
	high_priority.png
	low_priority.png
	only_seed.png
	icon.png        
	login.js 
	fetch_xml.js 
	torrent_details.js
	torrent_list.js
	action.js
	settings.js)

install(FILES ${common_data} DESTINATION ${DATA_INSTALL_DIR}/ktorrent/www/common)
