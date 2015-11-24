set(ktlogviewerplugin_SRC logviewerplugin.cpp logflags.cpp logviewer.cpp logprefpage.cpp logflagsdelegate.cpp)

ki18n_wrap_ui(ktlogviewerplugin_SRC logprefwidget.ui)
kconfig_add_kcfg_files(ktlogviewerplugin_SRC logviewerpluginsettings.kcfgc)
add_library(ktlogviewerplugin MODULE ${ktlogviewerplugin_SRC})

target_link_libraries(
    ktlogviewerplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    KF5::Parts
    )
install(TARGETS ktlogviewerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktlogviewerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

