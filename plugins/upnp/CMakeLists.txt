set(ktupnpplugin_SRC upnpplugin.cpp upnpwidget.cpp routermodel.cpp)

ki18n_wrap_ui(ktupnpplugin_SRC upnpwidget.ui)
kconfig_add_kcfg_files(ktupnpplugin_SRC upnppluginsettings.kcfgc)
add_library(ktupnpplugin MODULE ${ktupnpplugin_SRC})

target_link_libraries(
    ktupnpplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    KF5::Parts
    )
install(TARGETS ktupnpplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktupnpplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

