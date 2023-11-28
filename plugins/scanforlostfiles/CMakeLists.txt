ktorrent_add_plugin(ScanForLostFilesPlugin)

target_sources(ScanForLostFilesPlugin PRIVATE
	scanforlostfilesplugin.cpp
	scanforlostfileswidget.cpp
	scanforlostfilesprefpage.cpp
	scanforlostfilesthread.cpp
	nodeoperations.cpp)

ki18n_wrap_ui(ScanForLostFilesPlugin scanforlostfilesprefpage.ui scanforlostfileswidget.ui)
kconfig_add_kcfg_files(ScanForLostFilesPlugin scanforlostfilespluginsettings.kcfgc)

target_link_libraries(
    ScanForLostFilesPlugin
    ktcore
    KTorrent6
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOCore
)
