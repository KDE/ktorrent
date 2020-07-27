set(ktscanforlostfilesplugin_SRC
	scanforlostfilesplugin.cpp
	scanforlostfileswidget.cpp
	scanforlostfilesprefpage.cpp
	scanforlostfilesthread.cpp
	nodeoperations.cpp)

ki18n_wrap_ui(ktscanforlostfilesplugin_SRC scanforlostfilesprefpage.ui scanforlostfileswidget.ui)
kconfig_add_kcfg_files(ktscanforlostfilesplugin_SRC scanforlostfilespluginsettings.kcfgc)
add_library(ktorrent_scanforlostfiles MODULE ${ktscanforlostfilesplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_scanforlostfiles ktorrent_scanforlostfiles.desktop)

target_link_libraries(
    ktorrent_scanforlostfiles
    ktcore
    KF5::Torrent
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
)
install(TARGETS ktorrent_scanforlostfiles  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )

