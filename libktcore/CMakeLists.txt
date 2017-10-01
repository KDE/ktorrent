configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-ktcore.h.cmake ${CMAKE_BINARY_DIR}/config-ktcore.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(libktorrent_SRC 
	util/mmapfile.cpp
	util/itemselectionmodel.cpp
	util/stringcompletionmodel.cpp
	util/treefiltermodel.cpp
	
	interfaces/functions.cpp
	interfaces/plugin.cpp
	interfaces/guiinterface.cpp
	interfaces/coreinterface.cpp
	interfaces/prefpageinterface.cpp
	interfaces/activity.cpp
	interfaces/torrentactivityinterface.cpp
	
	torrent/queuemanager.cpp
	torrent/magnetmanager.cpp
	torrent/torrentfilemodel.cpp
	torrent/torrentfiletreemodel.cpp
	torrent/torrentfilelistmodel.cpp
	torrent/chunkbar.cpp 
	torrent/chunkbarrenderer.cpp
	torrent/jobtracker.cpp
	torrent/jobprogresswidget.cpp 
	torrent/basicjobprogresswidget.cpp 
	
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
	dbus/dbustorrentfilestream.cpp
	
	gui/centralwidget.cpp
	gui/tabbarwidget.cpp
	gui/extender.cpp
	
	plugin/pluginmanager.cpp
	plugin/pluginactivity.cpp

	ktorrent.kcfg
)

set(libktorrent_UI torrent/basicjobprogresswidget.ui )

ki18n_wrap_ui(libktorrent_SRC ${libktorrent_UI})
kconfig_add_kcfg_files(libktorrent_SRC settings.kcfgc)
add_library(ktcore ${libktorrent_SRC})
generate_export_header(ktcore BASE_NAME ktcore)

target_link_libraries(ktcore PUBLIC
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KCMUtils
    KF5::Parts
    KF5::Solid
    KF5::Torrent
    KF5::XmlGui
)

target_include_directories(ktcore PUBLIC "$<BUILD_INTERFACE:${KTORRENT_SOURCE_DIR}/libktcore;${KTORRENT_BINARY_DIR}/libktcore;${KTORRENT_BINARY_DIR}>")

set_target_properties(ktcore PROPERTIES VERSION 16.0.0 SOVERSION 16 )
install(TARGETS ktcore  ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

