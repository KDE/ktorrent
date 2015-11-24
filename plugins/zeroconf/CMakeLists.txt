set(ktzeroconfplugin_SRC zeroconfplugin.cpp torrentservice.cpp)

# ki18n_wrap_ui(ktzeroconfplugin_SRC zeroconfpref.ui searchbar.ui)
#kconfig_add_kcfg_files(ktzeroconfplugin_SRC zeroconfpluginsettings.kcfgc)
add_library(ktzeroconfplugin MODULE ${ktzeroconfplugin_SRC})

target_link_libraries(ktzeroconfplugin ${LIBKTORRENT_LIBRARIES} ktcore KF5::DNSSD KF5::Parts)
install(TARGETS ktzeroconfplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktzeroconfplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

