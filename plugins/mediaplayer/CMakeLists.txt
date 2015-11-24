find_package(Taglib REQUIRED)
include_directories(${TAGLIB_INCLUDES})

set(ktmediaplayerplugin_SRC 
	mediacontroller.cpp 
	playlist.cpp 
	playlistwidget.cpp 
	mediaplayeractivity.cpp 
	mediaplayerplugin.cpp 
	mediaview.cpp 
	mediamodel.cpp 
	mediafile.cpp 
	mediafilestream.cpp 
	mediaplayer.cpp 
	videowidget.cpp
	videochunkbar.cpp
)

set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt5_add_dbus_interface(ktmediaplayerplugin_SRC ${screensaver_xml} screensaver_interface)

ki18n_wrap_ui(ktmediaplayerplugin_SRC mediacontroller.ui)
kconfig_add_kcfg_files(ktmediaplayerplugin_SRC mediaplayerpluginsettings.kcfgc)
add_library(ktmediaplayerplugin MODULE ${ktmediaplayerplugin_SRC})

target_link_libraries(
    ktmediaplayerplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    KF5::KDELibs4Support
    KF5::Parts
    Phonon::phonon4qt5
    Qt5::Core
    Qt5::Gui
    )
install(TARGETS ktmediaplayerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktmediaplayerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktmediaplayerpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
