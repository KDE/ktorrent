set(ktipfilterplugin_SRC antip2p.cpp ipblockingprefpage.cpp convertthread.cpp convertdialog.cpp ipfilterplugin.cpp downloadandconvertjob.cpp)

kde4_add_ui_files(ktipfilterplugin_SRC ipblockingprefpage.ui convertdialog.ui)
kde4_add_kcfg_files(ktipfilterplugin_SRC ipfilterpluginsettings.kcfgc)
kde4_add_plugin(ktipfilterplugin ${ktipfilterplugin_SRC})

target_link_libraries(
    ktipfilterplugin
    ktcore
    btcore
    ${KDE4_KPARTS_LIBS}
    ${QT_QT3SUPPORT_LIBRARY}
    )
install(TARGETS ktipfilterplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktipfilterplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

