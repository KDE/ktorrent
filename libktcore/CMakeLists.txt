configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-ktcore.h.cmake ${CMAKE_BINARY_DIR}/config-ktcore.h)

include_directories(${CMAKE_BINARY_DIR})

add_library(ktcore)
set_target_properties(ktcore PROPERTIES
    VERSION 16.0.0
    SOVERSION 16
)

target_sources(ktcore PRIVATE
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
	groups/grouptreemodel.cpp
	
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

ki18n_wrap_ui(ktcore torrent/basicjobprogresswidget.ui)
kconfig_add_kcfg_files(ktcore settings.kcfgc)
generate_export_header(ktcore BASE_NAME ktcore)

target_link_libraries(ktcore PUBLIC
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtils
    KF6::Parts
    KF6::Solid
    KTorrent6
    KF6::XmlGui
)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

target_include_directories(ktcore PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libktcore>")

install(TARGETS ktcore  ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

