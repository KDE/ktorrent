ktorrent_add_plugin(ktorrent_scanfolder)

target_sources(ktorrent_scanfolder PRIVATE
	scanthread.cpp 
	torrentloadqueue.cpp 
	scanfolder.cpp 
	scanfolderplugin.cpp 
	scanfolderprefpage.cpp)

ki18n_wrap_ui(ktorrent_scanfolder scanfolderprefpage.ui)
kconfig_add_kcfg_files(ktorrent_scanfolder scanfolderpluginsettings.kcfgc)

target_link_libraries(
    ktorrent_scanfolder
    ktcore
    KF5::Torrent
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
)
