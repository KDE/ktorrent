set(ktzeroconfplugin_SRC zeroconfplugin.cpp torrentservice.cpp)

# ki18n_wrap_ui(ktzeroconfplugin_SRC zeroconfpref.ui searchbar.ui)
#kconfig_add_kcfg_files(ktzeroconfplugin_SRC zeroconfpluginsettings.kcfgc)
add_library(ktorrent_zeroconf MODULE ${ktzeroconfplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_zeroconf ktorrent_zeroconf.desktop)

target_link_libraries(ktorrent_zeroconf
    ktcore
    KF5::Torrent
    KF5::CoreAddons
    KF5::DNSSD
    KF5::I18n
)
install(TARGETS ktorrent_zeroconf  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )

