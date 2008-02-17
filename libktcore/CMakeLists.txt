include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(libktorrent_SRC 
	util/mmapfile.cpp	
	
	interfaces/functions.cpp
	interfaces/plugin.cpp
	interfaces/guiinterface.cpp
	interfaces/coreinterface.cpp
	interfaces/prefpageinterface.cpp
	
	torrent/queuemanager.cpp
	torrent/torrentfilemodel.cpp
	torrent/torrentfiletreemodel.cpp
	torrent/torrentfilelistmodel.cpp
	
	groups/group.cpp
	groups/torrentgroup.cpp
	groups/allgroup.cpp
	groups/downloadgroup.cpp
	groups/uploadgroup.cpp
	groups/activedownloadsgroup.cpp  
	groups/activegroup.cpp  
	groups/activeuploadsgroup.cpp
	groups/inactivedownloadsgroup.cpp  
	groups/inactivegroup.cpp  
	groups/inactiveuploadsgroup.cpp
	groups/queueddownloadsgroup.cpp  
	groups/queueduploadsgroup.cpp  
	groups/userdownloadsgroup.cpp  
	groups/useruploadsgroup.cpp
	groups/groupmanager.cpp

	ktorrent.kcfg
	pluginmanager.cpp
	pluginmanagerprefpage.cpp
)

# set(libktorrent_UI pluginmanagerwidget.ui )

kde4_add_ui_files(libktorrent_SRC ${libktorrent_UI})
kde4_add_kcfg_files(libktorrent_SRC settings.kcfgc)
kde4_add_library(ktcore SHARED ${libktorrent_SRC})

target_link_libraries(ktcore ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KUTILS_LIBS} btcore ${KDE4_SOLID_LIBS} ${GMP_LIBRARIES})
set_target_properties(ktcore PROPERTIES VERSION 3.0.0 SOVERSION 3 )
install(TARGETS ktcore  DESTINATION ${LIB_INSTALL_DIR} )

