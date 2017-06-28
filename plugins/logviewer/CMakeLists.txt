set(ktlogviewerplugin_SRC logviewerplugin.cpp logflags.cpp logviewer.cpp logprefpage.cpp logflagsdelegate.cpp)

ki18n_wrap_ui(ktlogviewerplugin_SRC logprefwidget.ui)
kconfig_add_kcfg_files(ktlogviewerplugin_SRC logviewerpluginsettings.kcfgc)
add_library(ktorrent_logviewer MODULE ${ktlogviewerplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_logviewer ktorrent_logviewer.desktop)

target_link_libraries(
    ktorrent_logviewer
    ktcore
    KF5::Torrent
    KF5::ConfigCore
    KF5::I18n
    KF5::XmlGui
)
install(TARGETS ktorrent_logviewer DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )

