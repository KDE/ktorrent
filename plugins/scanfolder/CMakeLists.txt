ktorrent_add_plugin(ScanFolderPlugin)

target_sources(ScanFolderPlugin PRIVATE
	scanthread.cpp 
	torrentloadqueue.cpp 
	scanfolder.cpp 
	scanfolderplugin.cpp 
	scanfolderprefpage.cpp)

ki18n_wrap_ui(ScanFolderPlugin scanfolderprefpage.ui)
kconfig_add_kcfg_files(ScanFolderPlugin scanfolderpluginsettings.kcfgc)

target_link_libraries(
    ScanFolderPlugin
    ktcore
    KTorrent6
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOCore
)
