project(KTORRENT)
find_package(KDE4 REQUIRED)
find_package(KDE4Internal REQUIRED)
find_package(GMP REQUIRED)
find_package(QCA2 REQUIRED)

# set some default settings
include(KDE4Defaults)

# make some more macros available
include (MacroLibrary)
#
# checks for functions and stuff
#
include(CheckIncludeFiles)
include(CheckFunctionExists)

# check for 64 bit file I/O functions
check_function_exists(fopen64 HAVE_FOPEN64)
check_function_exists(fseeko64 HAVE_FSEEKO64)
check_function_exists(ftello64 HAVE_FTELLO64)
check_function_exists(fstat64 HAVE_FSTAT64)
check_function_exists(stat64 HAVE_STAT64)
check_function_exists(ftruncate64 HAVE_FTRUNCATE64)
check_function_exists(lseek64 HAVE_LSEEK64)
check_function_exists(mmap64 HAVE_MMAP64)
check_function_exists(munmap64 HAVE_MUNMAP64)
check_function_exists(posix_fallocate64 HAVE_POSIX_FALLOCATE64)
check_function_exists(posix_fallocate HAVE_POSIX_FALLOCATE)
check_function_exists(statvfs HAVE_STATVFS)
check_function_exists(statvfs64 HAVE_STATVFS64)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-ktorrent.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktorrent.h)

add_definitions(${KDE4_ENABLE_EXCEPTIONS} ${KDE4_DEFINITIONS} )

option(ENABLE_DHT_SUPPORT "Whether to build KTorrent with DHT support or not" true)
if (ENABLE_DHT_SUPPORT)
  message(STATUS "Building KTorrent with DHT support (-DENABLE_DHT_SUPPORT=false to disable)")
  add_definitions(-DENABLE_DHT_SUPPORT)
endif (ENABLE_DHT_SUPPORT)

# option(ENABLE_GEOIP "Wether or not to enable support for the GeoIP library" true)
#if (ENABLE_GEOIP)
#  message(STATUS "Building KTorrent with GeoIP support")
#endif (ENABLE_GEOIP)



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
# add_subdirectory(test)
