project(kt4)
find_package(KDE4 REQUIRED)
find_package(KDE4Internal REQUIRED)

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}") 
set(CMAKE_SKIP_RPATH)
include_directories( ${KDE4_INCLUDES} ${CMAKE_BINARY_DIR})

add_definitions( ${KDE4_DEFINITIONS})
# link_directories( ${CMAKE_BINARY_DIR}/lib)


add_subdirectory(libktorrent)
add_subdirectory(plugins)
add_subdirectory(ideal)
add_subdirectory(ktorrent)
# add_subdirectory(ktupnptest)
# add_subdirectory(kttorinfo)

