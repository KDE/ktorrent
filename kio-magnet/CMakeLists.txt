project(kio-magnet)
find_package(Qt4 4.5.0 REQUIRED)
find_package(KDE4 4.3.0 REQUIRED) # hopefully this is ok

# set some default settings
include(KDE4Defaults)

# make some more macros available
include (MacroLibrary)

#
# checks for functions and stuff
#
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckTypeSize) #XFS

# according to http://www.cmake.org/pipermail/cmake/2008-June/022381.html
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
add_definitions( ${KDE4_DEFINITIONS} )
include_directories( ${KDE4_INCLUDES} )


########### kio_magnet ###############

set(kio_magnet_SRCS kio_magnet.cpp dbushandler.cpp dbusthread.cpp 
                     interfaces/ktorrentcoreinterface.cpp interfaces/ktorrenttorrentinterface.cpp
                     interfaces/ktorrenttorrentfilestreaminterface.cpp interfaces/ktorrentgroupinterface.cpp )


kde4_add_kcfg_files(kio_magnet_SRCS magnetsettings.kcfgc)
install(FILES magnetsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})

kde4_add_plugin(kio_magnet ${kio_magnet_SRCS})

target_link_libraries(kio_magnet ${KDE4_KIO_LIBS} )

# set(ktorrent_torrent_xml ./interfaces/org.ktorrent.torrent.xml)
#qt4_add_dbus_interfaces(kio_magnet_SRCS org.ktorrent.core.xml org.ktorrent.torrent.xml )
# qt4_add_dbus_interface(kio_magnet_SRCS org.ktorrent.core.xml ktorrent_torrent_interface )
# PKGCONFIG_GETVAR(dbus-1 prefix DBUS_PREFIX)
# set(ktorrent_core_xml ./interfaces/org.ktorrent.core.xml)
#set(sso_xml ${DBUS_PREFIX}/share/dbus-1/interfaces/com.ubuntu.sso.xml)
# set_source_files_properties(${ktorrent_core_xml} PROPERTIES NO_NAMESPACE TRUE)
# qt4_add_dbus_interface(kio_magnet_SRCS ${ktorrent_core_xml} KTorrentCoreInterface)

install(TARGETS kio_magnet DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES magnet.protocol DESTINATION  ${SERVICES_INSTALL_DIR} )


#include (CPack)
#set(CPACK_CMAKE_GENERATOR "Unix Makefiles")
#set(${CPACK_GENERATOR} "TGZ;DEB")
#set(CPACK_INSTALL_CMAKE_PROJECTS "kio-magnet")
#set(CPACK_PACKAGE_VERSION_MAJOR "0")
#set(CPACK_PACKAGE_VERSION_MINOR "0")
#set(CPACK_PACKAGE_VERSION_PATCH "1")
#set(CPACK_PACKAGE_NAME "kio-magnet")
#set(CPACK_PACKAGE_VERSION "0.0.1")
#set(CPACK_DEBIAN_PACKAGE_VERSION "0.0.1~ubuntu0")
#set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "kdelibs5 (>= 4.3.0)  ktorrent (>= 4.0.3)")
#set(CPACK_DEBIAN_BUILD_DEPENDS "kdelibs5-dev (>= 4.3.0)")
#set(CPACK_PACKAGE_CONTACT "ch_weil@web.de")
#set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A kioslave for magnet-urls.")
#set(CPACK_DEBIAN_PACKAGE_SECTION "kde")
#set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
#set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "konqueror")
