# Add an option to compile & link against system GeoIP
option(BUILD_WITH_GEOIP
    "Link InfoWidget plugin against system GeoIP library and use system-wide default GeoIP Country database"
    ON)

if (BUILD_WITH_GEOIP)
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
    find_package(MaxmindDB)
    set_package_properties(MaxmindDB PROPERTIES
        TYPE OPTIONAL
        PURPOSE "Required for KTorrent's infowidget plugin"
    )
    if (MaxmindDB_FOUND)
        message(STATUS "   Linking InfoWidget against system GeoIP library")
        set(geoip_link ${MaxmindDB_LIBRARIES})
    else()
        set(BUILD_WITH_GEOIP OFF CACHE BOOL
          "GeoIP development files could not be found on this system. Forcing this option to OFF"
          FORCE)
        message(WARNING "GeoIP library development files could not be found on your system.")
    endif()
endif()

ktorrent_add_plugin(InfoWidgetPlugin)

target_sources(InfoWidgetPlugin PRIVATE
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
    target_sources(InfoWidgetPlugin PRIVATE geoipmanager.cpp)
    add_definitions(-DBUILD_WITH_GEOIP=1)
else()
    add_definitions(-DBUILD_WITH_GEOIP=0)
endif()

ki18n_wrap_ui(InfoWidgetPlugin iwprefpage.ui statustab.ui chunkdownloadview.ui trackerview.ui webseedstab.ui)
kconfig_add_kcfg_files(InfoWidgetPlugin infowidgetpluginsettings.kcfgc)

target_link_libraries(
    InfoWidgetPlugin
    ktcore
    KTorrent6
    KF6::ConfigCore
    KF6::I18n
    KF6::KIOFileWidgets
    KF6::KIOWidgets
    KF6::WidgetsAddons
    ${geoip_link}
)
