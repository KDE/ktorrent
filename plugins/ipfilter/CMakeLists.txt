ktorrent_add_plugin(ktorrent_ipfilter)

find_package(KF6Archive ${KF_MIN_VERSION} REQUIRED)

target_sources(ktorrent_ipfilter PRIVATE
    ipblocklist.cpp
    ipblockingprefpage.cpp
    convertthread.cpp
    convertdialog.cpp
    ipfilterplugin.cpp
    downloadandconvertjob.cpp
)

ki18n_wrap_ui(ktorrent_ipfilter ipblockingprefpage.ui convertdialog.ui)
kconfig_add_kcfg_files(ktorrent_ipfilter ipfilterpluginsettings.kcfgc)

target_link_libraries(
    ktorrent_ipfilter
    ktcore
    KF5::Torrent
    KF6::Archive
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::Notifications
    KF6::TextWidgets
    KF6::WidgetsAddons
)

find_package(Qt6Test ${QT_MIN_VERSION})
if (Qt6Test_DIR)
    add_subdirectory(tests)
endif()
