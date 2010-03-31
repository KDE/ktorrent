include_directories(${KTORRENT_SOURCE_DIR}/libktupnp)

set(ktupnpplugin_SRC upnpplugin.cpp upnpwidget.cpp routermodel.cpp)

kde4_add_ui_files(ktupnpplugin_SRC upnpwidget.ui)
kde4_add_kcfg_files(ktupnpplugin_SRC upnppluginsettings.kcfgc)
kde4_add_plugin(ktupnpplugin ${ktupnpplugin_SRC})

target_link_libraries(
    ktupnpplugin
    ktcore
    ktupnp
    btcore
    ${KDE4_KPARTS_LIBS}
    )
install(TARGETS ktupnpplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktupnpplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

