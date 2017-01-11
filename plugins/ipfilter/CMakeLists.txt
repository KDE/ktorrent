set(ktipfilterplugin_SRC ipblocklist.cpp ipblockingprefpage.cpp convertthread.cpp convertdialog.cpp ipfilterplugin.cpp downloadandconvertjob.cpp)

ki18n_wrap_ui(ktipfilterplugin_SRC ipblockingprefpage.ui convertdialog.ui)
kconfig_add_kcfg_files(ktipfilterplugin_SRC ipfilterpluginsettings.kcfgc)
add_library(ktorrent_ipfilter MODULE ${ktipfilterplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_ipfilter ktorrent_ipfilter.desktop)


target_link_libraries(
    ktorrent_ipfilter
    ktcore
    KF5::Torrent 
    KF5::Parts
    KF5::KDELibs4Support
    )
install(TARGETS ktorrent_ipfilter  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )

# add_subdirectory(tests)
