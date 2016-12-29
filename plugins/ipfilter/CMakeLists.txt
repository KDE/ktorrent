set(ktipfilterplugin_SRC ipblocklist.cpp ipblockingprefpage.cpp convertthread.cpp convertdialog.cpp ipfilterplugin.cpp downloadandconvertjob.cpp)

ki18n_wrap_ui(ktipfilterplugin_SRC ipblockingprefpage.ui convertdialog.ui)
kconfig_add_kcfg_files(ktipfilterplugin_SRC ipfilterpluginsettings.kcfgc)
# [Fonic]
#add_library(ktipfilterplugin MODULE ${ktipfilterplugin_SRC})
add_library(ktorrent_ipfilter MODULE ${ktipfilterplugin_SRC})

# [Fonic]
kcoreaddons_desktop_to_json(ktorrent_ipfilter ktorrent_ipfilter.desktop)

target_link_libraries(
    # [Fonic]
    #ktipfilterplugin
    ktorrent_ipfilter
    ktcore
    KF5::Torrent 
    KF5::Parts
    # [Fonic] Required for ipfilterplugin.h/.cpp, void IPFilterPlugin::notification(const QString& msg)
    KF5::Notifications
    )
# [Fonic]
#install(TARGETS ktipfilterplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
#install(FILES ktipfilterplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(TARGETS ktorrent_ipfilter  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )

# [Fonic] test also ported, but not sure if it works as it should
add_subdirectory(tests)
