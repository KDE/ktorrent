find_package(Taglib REQUIRED)
include_directories(${TAGLIB_INCLUDES})

set(ktmediaplayerplugin_SRC mediaplayerplugin.cpp mediaview.cpp mediamodel.cpp mediaplayer.cpp videowidget.cpp)

#kde4_add_ui_files(ktmediaplayerplugin_SRC mediaplayerpref.ui)
kde4_add_kcfg_files(ktmediaplayerplugin_SRC mediaplayerpluginsettings.kcfgc)
kde4_add_plugin(ktmediaplayerplugin ${ktmediaplayerplugin_SRC})

target_link_libraries(
    ktmediaplayerplugin
    ktcore
    btcore
	${TAGLIB_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_PHONON_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )
	
install(TARGETS ktmediaplayerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktmediaplayerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktmediaplayerpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
