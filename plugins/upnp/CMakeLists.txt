ktorrent_add_plugin(UPnPPlugin)

target_sources(UPnPPlugin PRIVATE
    upnpplugin.cpp
    upnpwidget.cpp
    routermodel.cpp
)

ki18n_wrap_ui(UPnPPlugin upnpwidget.ui)
kconfig_add_kcfg_files(UPnPPlugin upnppluginsettings.kcfgc)

target_link_libraries(
    UPnPPlugin
    ktcore
    KTorrent6
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::WidgetsAddons
)
