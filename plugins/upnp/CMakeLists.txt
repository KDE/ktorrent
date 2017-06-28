set(ktupnpplugin_SRC upnpplugin.cpp upnpwidget.cpp routermodel.cpp)

ki18n_wrap_ui(ktupnpplugin_SRC upnpwidget.ui)
kconfig_add_kcfg_files(ktupnpplugin_SRC upnppluginsettings.kcfgc)
add_library(ktorrent_upnp MODULE ${ktupnpplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_upnp ktorrent_upnp.desktop)

target_link_libraries(
    ktorrent_upnp
    ktcore
    KF5::Torrent
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
)
install(TARGETS ktorrent_upnp  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )

