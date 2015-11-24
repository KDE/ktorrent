set(ktipfilterplugin_SRC ipblocklist.cpp ipblockingprefpage.cpp convertthread.cpp convertdialog.cpp ipfilterplugin.cpp downloadandconvertjob.cpp)

ki18n_wrap_ui(ktipfilterplugin_SRC ipblockingprefpage.ui convertdialog.ui)
kconfig_add_kcfg_files(ktipfilterplugin_SRC ipfilterpluginsettings.kcfgc)
add_library(ktipfilterplugin MODULE ${ktipfilterplugin_SRC})

target_link_libraries(
    ktipfilterplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    KF5::Parts
    )
install(TARGETS ktipfilterplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktipfilterplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


add_subdirectory(tests)
