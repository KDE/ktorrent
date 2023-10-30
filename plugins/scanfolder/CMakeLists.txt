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
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOCore
)
