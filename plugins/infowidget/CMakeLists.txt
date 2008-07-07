# Add an option to compile & link against system GeoIP (off by default)
option(WITH_SYSTEM_GEOIP 
	"Link InfoWidget plugin against system GeoIP library and use system-wide default GeoIP Country database"
	OFF)

if (WITH_SYSTEM_GEOIP)
	include(CheckIncludeFile)
	check_include_file("GeoIP.h" have_geoip_include)
	find_library(SYSTEM_GEOIP_LIBRARY "GeoIP")
	check_library_exists("GeoIP" "GeoIP_open_type" ${SYSTEM_GEOIP_LIBRARY} have_geoip_library)
	if (NOT have_geoip_include OR NOT have_geoip_library)
		set(WITH_SYSTEM_GEOIP OFF CACHE BOOL
		  "GeoIP development files could not be found on this system. Forcing this option to OFF"
		  FORCE)
		message(SEND_ERROR
		  "GeoIP library development files could not be found on your system. Embedding a local copy of GeoIP instead")
	endif(NOT have_geoip_include OR NOT have_geoip_library)
endif(WITH_SYSTEM_GEOIP)

if (WITH_SYSTEM_GEOIP)
	message(STATUS "   Linking InfoWidget against system GeoIP library")
	add_definitions( -DUSE_SYSTEM_GEOIP )
	set(geoip_link "GeoIP")
else(WITH_SYSTEM_GEOIP)
	message(STATUS "   Compiling GeoIP support directly into InfoWidget plugin")
	set(geoip_src GeoIP.c)
endif(WITH_SYSTEM_GEOIP)    

add_subdirectory(geoip)

set(ktinfowidgetplugin_SRC 
	infowidgetplugin.cpp 
	iwprefpage.cpp 
	monitor.cpp 
	availabilitychunkbar.cpp 
	downloadedchunkbar.cpp 
	statustab.cpp
	fileview.cpp
	peerview.cpp
	peerviewmodel.cpp
	chunkdownloadview.cpp
	chunkdownloadmodel.cpp
	trackerview.cpp
	trackermodel.cpp
	${geoip_src}
	flagdb.cpp
	iwfiletreemodel.cpp
	iwfilelistmodel.cpp
	webseedstab.cpp
	webseedsmodel.cpp)

kde4_add_ui_files(ktinfowidgetplugin_SRC iwprefpage.ui statustab.ui chunkdownloadview.ui trackerview.ui webseedstab.ui)
kde4_add_kcfg_files(ktinfowidgetplugin_SRC infowidgetpluginsettings.kcfgc)
kde4_add_plugin(ktinfowidgetplugin ${ktinfowidgetplugin_SRC})

target_link_libraries(ktinfowidgetplugin ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KPARTS_LIBS} ktcore ${geoip_link})
install(TARGETS ktinfowidgetplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktinfowidgetplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

