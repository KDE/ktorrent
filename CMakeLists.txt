project(KTORRENT)
find_package(Qt4 4.7.0 REQUIRED)
find_package(KDE4 4.4.0 REQUIRED)
find_package(KTorrent REQUIRED)
find_package(Boost REQUIRED)

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
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
set (KTORRENT_DBUS_XML_DIR ${KTORRENT_SOURCE_DIR}/dbus_xml)

include_directories( ${KDE4_INCLUDES} 
   ${KTORRENT_BINARY_DIR}
   ${KTORRENT_BINARY_DIR}/libktcore
   ${KTORRENT_SOURCE_DIR}/libktcore
   ${LIBKTORRENT_INCLUDE_DIR}
   ${KTORRENT_SOURCE_DIR}
   ${GMP_INCLUDE_DIR}
   ${Boost_INCLUDE_DIR}
   )

add_subdirectory(libktcore)

option(ENABLE_KIO_MAGNET "Whether to build kio-magnet or not" false)
if (${ENABLE_KIO_MAGNET})
    message(STATUS "Building kio-magnet (-DENABLE_KIO_MAGNET=false to disable)")
    add_subdirectory(kio-magnet)
else (${ENABLE_KIO_MAGNET})
    message(STATUS "Not building kio-magnet (-DENABLE_KIO_MAGNET=true to enable)")
endif (${ENABLE_KIO_MAGNET})

add_subdirectory(plugins)
add_subdirectory(ktorrent)
add_subdirectory(ktupnptest)
add_subdirectory(plasma)
add_subdirectory(ktmagnetdownloader)
add_subdirectory(doc)



# --- show log ---
macro_display_feature_log()


