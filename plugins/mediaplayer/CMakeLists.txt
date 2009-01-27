set(ktmediaplayerplugin_SRC mediaplayerplugin.cpp mediaview.cpp mediamodel.cpp mediaplayer.cpp videowidget.cpp)

#kde4_add_ui_files(ktmediaplayerplugin_SRC mediaplayerpref.ui)
kde4_add_kcfg_files(ktmediaplayerplugin_SRC mediaplayerpluginsettings.kcfgc)
kde4_add_plugin(ktmediaplayerplugin ${ktmediaplayerplugin_SRC})

target_link_libraries(ktmediaplayerplugin ${KDE4_PHONON_LIBS} ktcore btcore)
install(TARGETS ktmediaplayerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktmediaplayerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktmediaplayerpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
