project(KTORRENT)
find_package(KDE4 REQUIRED)
find_package(KDE4Internal REQUIRED)
find_package(GMP REQUIRED)
find_package(QCA2 REQUIRED)

# backward compatibility to <= KDE 4.1
MACRO_ENSURE_VERSION( "4.1.83" ${KDE_VERSION} KDE_VERSION_OK)
if(NOT KDE_VERSION_OK)
  macro_optional_find_package(Plasma)
  macro_log_feature(PLASMA_FOUND "Plasma" "Plasma libraries and header files" "http://plasma.kde.org/" FALSE "" "Plasma is needed for KTorrent Plasmoid")
else(NOT KDE_VERSION_OK)
  set(PLASMA_FOUND true)
  set(PLASMA_LIBS ${KDE4_PLASMA_LIBS})
endif(NOT KDE_VERSION_OK)

# set some default settings
include(KDE4Defaults)
enable_testing()
set(BUILD_TESTING ON)

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

option(ENABLE_DHT_SUPPORT "Whether to build KTorrent with DHT support or not" true)
if (ENABLE_DHT_SUPPORT)
  message(STATUS "Building KTorrent with DHT support (-DENABLE_DHT_SUPPORT=false to disable)")
  add_definitions(-DENABLE_DHT_SUPPORT)
endif (ENABLE_DHT_SUPPORT)

# option(ENABLE_GEOIP "Wether or not to enable support for the GeoIP library" true)
#if (ENABLE_GEOIP)
#  message(STATUS "Building KTorrent with GeoIP support")
#endif (ENABLE_GEOIP)

set (KTORRENT_DBUS_XML_DIR ${KTORRENT_SOURCE_DIR}/dbus_xml)

include_directories( ${KDE4_INCLUDES} 
   ${KTORRENT_BINARY_DIR}
   ${KTORRENT_BINARY_DIR}/libktcore
   ${KTORRENT_SOURCE_DIR}/libktcore
   ${KTORRENT_BINARY_DIR}/libbtcore
   ${KTORRENT_SOURCE_DIR}/libbtcore
   ${KTORRENT_SOURCE_DIR}
   ${GMP_INCLUDE_DIR}
   )

add_subdirectory(cmake)
add_subdirectory(libbtcore)
add_subdirectory(libktcore)
add_subdirectory(libktupnp)
add_subdirectory(plugins)
add_subdirectory(ideal)
add_subdirectory(ktorrent)
add_subdirectory(ktupnptest)
if( PLASMA_FOUND )
    add_subdirectory(plasma)
endif(PLASMA_FOUND)
add_subdirectory(ktutpd)
add_subdirectory(ktmagnetdownloader)

# --- show log ---
macro_display_feature_log()


