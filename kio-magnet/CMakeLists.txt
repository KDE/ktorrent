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

install(TARGETS kio_magnet DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES magnet.protocol DESTINATION  ${SERVICES_INSTALL_DIR} )


