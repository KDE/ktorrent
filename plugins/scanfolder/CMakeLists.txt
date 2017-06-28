set(ktscanfolderplugin_SRC 
	scanthread.cpp 
	torrentloadqueue.cpp 
	scanfolder.cpp 
	scanfolderplugin.cpp 
	scanfolderprefpage.cpp)

ki18n_wrap_ui(ktscanfolderplugin_SRC scanfolderprefpage.ui)
kconfig_add_kcfg_files(ktscanfolderplugin_SRC scanfolderpluginsettings.kcfgc)
add_library(ktorrent_scanfolder MODULE ${ktscanfolderplugin_SRC})

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

