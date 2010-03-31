set(ktlogviewerplugin_SRC logviewerplugin.cpp logflags.cpp logviewer.cpp logprefpage.cpp logflagsdelegate.cpp)

kde4_add_ui_files(ktlogviewerplugin_SRC logprefwidget.ui)
kde4_add_kcfg_files(ktlogviewerplugin_SRC logviewerpluginsettings.kcfgc)
kde4_add_plugin(ktlogviewerplugin ${ktlogviewerplugin_SRC})

target_link_libraries(
    ktlogviewerplugin
    ktcore
    btcore
    ${KDE4_KPARTS_LIBS}
    )
install(TARGETS ktlogviewerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktlogviewerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

