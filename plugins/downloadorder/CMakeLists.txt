set(ktdownloadorderplugin_SRC downloadorderplugin.cpp downloadorderdialog.cpp downloadordermanager.cpp downloadordermodel.cpp)

ki18n_wrap_ui(ktdownloadorderplugin_SRC downloadorderwidget.ui)
#kconfig_add_kcfg_files(ktdownloadorderplugin_SRC searchpluginsettings.kcfgc)
add_library(ktdownloadorderplugin MODULE ${ktdownloadorderplugin_SRC})

target_link_libraries(
    ktdownloadorderplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    KF5::Parts
    )
install(TARGETS ktdownloadorderplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktdownloadorderplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktdownloadorderpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
