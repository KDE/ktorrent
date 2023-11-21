ktorrent_add_plugin(ktorrent_zeroconf)

target_sources(ktorrent_zeroconf PRIVATE
    zeroconfplugin.cpp
    torrentservice.cpp
)

# ki18n_wrap_ui(ktorrent_zeroconf zeroconfpref.ui searchbar.ui)
#kconfig_add_kcfg_files(ktorrent_zeroconf zeroconfpluginsettings.kcfgc)

target_link_libraries(ktorrent_zeroconf
    ktcore
    KTorrent6
    KF6::CoreAddons
    KF6::DNSSD
    KF6::I18n
)
