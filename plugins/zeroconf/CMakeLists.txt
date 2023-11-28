ktorrent_add_plugin(ZeroconfPlugin)

target_sources(ZeroconfPlugin PRIVATE
    zeroconfplugin.cpp
    torrentservice.cpp
)

# ki18n_wrap_ui(ZeroconfPlugin zeroconfpref.ui searchbar.ui)
#kconfig_add_kcfg_files(ZeroconfPlugin zeroconfpluginsettings.kcfgc)

target_link_libraries(ZeroconfPlugin
    ktcore
    KTorrent6
    KF6::CoreAddons
    KF6::DNSSD
    KF6::I18n
)
