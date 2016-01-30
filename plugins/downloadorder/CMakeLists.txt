set(ktdownloadorderplugin_SRC downloadorderplugin.cpp downloadorderdialog.cpp downloadordermanager.cpp downloadordermodel.cpp)

ki18n_wrap_ui(ktdownloadorderplugin_SRC downloadorderwidget.ui)
#kconfig_add_kcfg_files(ktdownloadorderplugin_SRC searchpluginsettings.kcfgc)


add_library(ktorrent_downloadorder MODULE ${ktdownloadorderplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_downloadorder  ktorrent_downloadorder.desktop)

target_link_libraries(
    ktorrent_downloadorder
    ktcore
    ktorrent
    )
install(TARGETS ktorrent_downloadorder  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_downloadorderui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
