# Add an option to compile & link against system GeoIP (off by default)
option(WITH_SYSTEM_GEOIP 
	"Link InfoWidget plugin against system GeoIP library and use system-wide default GeoIP Country database"
	OFF)

if (WITH_SYSTEM_GEOIP)
	find_path(GEOIP_INCLUDE_DIR NAMES GeoIP.h PATHS / ${INCLUDE_INSTALL_DIR}/)
	find_library(GEOIP_LIBRARY NAMES GeoIP PATHS ${KDE4_LIB_DIR} ${LIB_INSTALL_DIR})
	
	if (NOT GEOIP_INCLUDE_DIR OR NOT GEOIP_LIBRARY)
		set(WITH_SYSTEM_GEOIP OFF CACHE BOOL
		  "GeoIP development files could not be found on this system. Forcing this option to OFF"
		  FORCE)
		message(SEND_ERROR
		  "GeoIP library development files could not be found on your system. Embedding a local copy of GeoIP instead")
	endif(NOT GEOIP_INCLUDE_DIR OR NOT GEOIP_LIBRARY)
endif(WITH_SYSTEM_GEOIP)

if (WITH_SYSTEM_GEOIP)
	message(STATUS "   Linking InfoWidget against system GeoIP library")
	add_definitions( -DUSE_SYSTEM_GEOIP )
	include_directories(GEOIP_INCLUDE_DIR)
	set(geoip_link ${GEOIP_LIBRARY})
else(WITH_SYSTEM_GEOIP)
	message(STATUS "   Compiling GeoIP support directly into InfoWidget plugin")
	set(geoip_src GeoIP.c)
endif(WITH_SYSTEM_GEOIP)    

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
	addtrackersdialog.cpp
	${geoip_src}
	geoipmanager.cpp
	flagdb.cpp
	iwfiletreemodel.cpp
	iwfilelistmodel.cpp
	webseedstab.cpp
	webseedsmodel.cpp)

ki18n_wrap_ui(ktinfowidgetplugin_SRC iwprefpage.ui statustab.ui chunkdownloadview.ui trackerview.ui webseedstab.ui)
kconfig_add_kcfg_files(ktinfowidgetplugin_SRC infowidgetpluginsettings.kcfgc)
add_library(ktorrent_infowidget MODULE ${ktinfowidgetplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_infowidget ktorrent_infowidget.desktop)

target_link_libraries(
    ktorrent_infowidget
    ktcore
    KF5::Torrent
    KF5::ConfigCore
    KF5::I18n
    KF5::KIOFileWidgets
    KF5::KIOWidgets
    KF5::WidgetsAddons
    ${geoip_link}
)
install(TARGETS ktorrent_infowidget DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )

