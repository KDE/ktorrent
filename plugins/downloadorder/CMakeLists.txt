set(ktdownloadorderplugin_SRC downloadorderplugin.cpp downloadorderdialog.cpp downloadordermanager.cpp downloadordermodel.cpp)

kde4_add_ui_files(ktdownloadorderplugin_SRC downloadorderwidget.ui)
#kde4_add_kcfg_files(ktdownloadorderplugin_SRC searchpluginsettings.kcfgc)
kde4_add_plugin(ktdownloadorderplugin ${ktdownloadorderplugin_SRC})

target_link_libraries(
    ktdownloadorderplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    )
install(TARGETS ktdownloadorderplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktdownloadorderplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktdownloadorderpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
