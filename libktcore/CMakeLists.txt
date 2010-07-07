configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-ktcore.h.cmake ${CMAKE_BINARY_DIR}/config-ktcore.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(libktorrent_SRC 
	util/mmapfile.cpp
	util/itemselectionmodel.cpp
	util/stringcompletionmodel.cpp
	util/hintlineedit.cpp
	util/treefiltermodel.cpp
	
	interfaces/functions.cpp
	interfaces/plugin.cpp
	interfaces/guiinterface.cpp
	interfaces/coreinterface.cpp
	interfaces/prefpageinterface.cpp
	interfaces/activity.cpp
	interfaces/torrentactivityinterface.cpp
	
	torrent/queuemanager.cpp
	torrent/torrentfilemodel.cpp
	torrent/torrentfiletreemodel.cpp
	torrent/torrentfilelistmodel.cpp
	torrent/chunkbar.cpp 
	torrent/chunkbarrenderer.cpp
	
	groups/group.cpp
	groups/torrentgroup.cpp
	groups/allgroup.cpp
	groups/ungroupedgroup.cpp
	groups/groupmanager.cpp
	groups/functiongroup.cpp
	
	dbus/dbus.cpp
	dbus/dbustorrent.cpp
	dbus/dbusgroup.cpp
	dbus/dbussettings.cpp
	
	gui/centralwidget.cpp
	gui/activitybar.cpp
	gui/activitylistmodel.cpp
	gui/activitylistwidget.cpp
	gui/activitylistdelegate.cpp
	gui/tabbarwidget.cpp
	
	plugin/pluginmanager.cpp
	plugin/pluginactivity.cpp

	ktorrent.kcfg
)

# set(libktorrent_UI pluginmanagerwidget.ui )

kde4_add_ui_files(libktorrent_SRC ${libktorrent_UI})
kde4_add_kcfg_files(libktorrent_SRC settings.kcfgc)
kde4_add_library(ktcore SHARED ${libktorrent_SRC})

target_link_libraries(ktcore ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KPARTS_LIBS} ${KDE4_KUTILS_LIBS} ${LIBKTORRENT_LIBRARIES} ${KDE4_SOLID_LIBS} ${GMP_LIBRARIES})
set_target_properties(ktcore PROPERTIES VERSION 11.0.1 SOVERSION 11 )
install(TARGETS ktcore  ${INSTALL_TARGETS_DEFAULT_ARGS} )

