add_library(ktorrent_scanfolder MODULE)

target_sources(ktorrent_scanfolder PRIVATE
	scanthread.cpp 
	torrentloadqueue.cpp 
	scanfolder.cpp 
	scanfolderplugin.cpp 
	scanfolderprefpage.cpp)

ki18n_wrap_ui(ktorrent_scanfolder scanfolderprefpage.ui)
kconfig_add_kcfg_files(ktorrent_scanfolder scanfolderpluginsettings.kcfgc)

kcoreaddons_desktop_to_json(ktorrent_scanfolder ktorrent_scanfolder.desktop)

target_link_libraries(
    ktorrent_scanfolder
    ktcore
    KF5::Torrent
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
)
install(TARGETS ktorrent_scanfolder  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )

