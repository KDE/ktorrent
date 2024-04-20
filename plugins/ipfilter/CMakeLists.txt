ktorrent_add_plugin(IPFilterPlugin)

find_package(KF6Archive ${KF_MIN_VERSION} REQUIRED)

target_sources(IPFilterPlugin PRIVATE
    ipblocklist.cpp
    ipblockingprefpage.cpp
    convertthread.cpp
    convertdialog.cpp
    ipfilterplugin.cpp
    downloadandconvertjob.cpp
)

ki18n_wrap_ui(IPFilterPlugin ipblockingprefpage.ui convertdialog.ui)
kconfig_add_kcfg_files(IPFilterPlugin ipfilterpluginsettings.kcfgc)

target_link_libraries(
    IPFilterPlugin
    ktcore
    KTorrent6
    KF6::Archive
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::Notifications
    KF6::TextWidgets
    KF6::WidgetsAddons
)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
