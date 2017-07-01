set(ktipfilterplugin_SRC ipblocklist.cpp ipblockingprefpage.cpp convertthread.cpp convertdialog.cpp ipfilterplugin.cpp downloadandconvertjob.cpp)

ki18n_wrap_ui(ktipfilterplugin_SRC ipblockingprefpage.ui convertdialog.ui)
kconfig_add_kcfg_files(ktipfilterplugin_SRC ipfilterpluginsettings.kcfgc)
add_library(ktorrent_ipfilter MODULE ${ktipfilterplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_ipfilter ktorrent_ipfilter.desktop)

target_link_libraries(
    ktorrent_ipfilter
    ktcore
    KF5::Torrent
    KF5::Archive
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOWidgets
    KF5::Notifications
    KF5::TextWidgets
    KF5::WidgetsAddons
)
install(TARGETS ktorrent_ipfilter DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )

find_package(Qt5Test ${QT5_REQUIRED_VERSION})
if (Qt5Test_DIR)
    add_subdirectory(tests)
endif()
