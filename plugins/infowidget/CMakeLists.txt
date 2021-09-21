# Add an option to compile & link against system GeoIP
option(BUILD_WITH_GEOIP
	"Link InfoWidget plugin against system GeoIP library and use system-wide default GeoIP Country database"
	ON)

if (BUILD_WITH_GEOIP)
	find_path(GEOIP_INCLUDE_DIR NAMES GeoIP.h PATHS / ${INCLUDE_INSTALL_DIR}/)
	find_library(GEOIP_LIBRARY NAMES GeoIP PATHS ${LIB_INSTALL_DIR})
	
	if (NOT GEOIP_INCLUDE_DIR OR NOT GEOIP_LIBRARY)
		set(BUILD_WITH_GEOIP OFF CACHE BOOL
		  "GeoIP development files could not be found on this system. Forcing this option to OFF"
		  FORCE)
		message(WARNING "GeoIP library development files could not be found on your system.")
	else()
        message(STATUS "   Linking InfoWidget against system GeoIP library")
        include_directories(GEOIP_INCLUDE_DIR)
        set(geoip_link ${GEOIP_LIBRARY})
    endif()
endif(BUILD_WITH_GEOIP)

kcoreaddons_add_plugin(ktorrent_infowidget INSTALL_NAMESPACE "ktorrent")

target_sources(ktorrent_infowidget PRIVATE
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
	flagdb.cpp
	iwfiletreemodel.cpp
	iwfilelistmodel.cpp
	webseedstab.cpp
	webseedsmodel.cpp)

if (BUILD_WITH_GEOIP)
    target_sources(ktorrent_infowidget PRIVATE geoipmanager.cpp)
    add_definitions(-DBUILD_WITH_GEOIP=1)
else()
    add_definitions(-DBUILD_WITH_GEOIP=0)
endif()

ki18n_wrap_ui(ktorrent_infowidget iwprefpage.ui statustab.ui chunkdownloadview.ui trackerview.ui webseedstab.ui)
kconfig_add_kcfg_files(ktorrent_infowidget infowidgetpluginsettings.kcfgc)

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
