find_package(Taglib REQUIRED)
include_directories(${TAGLIB_INCLUDES})

set(ktmediaplayerplugin_SRC 
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
QT4_ADD_DBUS_INTERFACE(ktmediaplayerplugin_SRC ${screensaver_xml} screensaver_interface)

#kde4_add_ui_files(ktmediaplayerplugin_SRC mediaplayerpref.ui)
kde4_add_kcfg_files(ktmediaplayerplugin_SRC mediaplayerpluginsettings.kcfgc)
kde4_add_plugin(ktmediaplayerplugin ${ktmediaplayerplugin_SRC})

target_link_libraries(
    ktmediaplayerplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
	${TAGLIB_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_PHONON_LIBS}
	${KDE4_SOLID_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )
install(TARGETS ktmediaplayerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktmediaplayerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktmediaplayerpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
