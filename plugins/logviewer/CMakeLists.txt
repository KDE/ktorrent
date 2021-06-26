add_library(ktorrent_logviewer MODULE)

target_sources(ktorrent_logviewer PRIVATE
    logviewerplugin.cpp
    logflags.cpp
    logviewer.cpp
    logprefpage.cpp
    logflagsdelegate.cpp
)

ki18n_wrap_ui(ktorrent_logviewer logprefwidget.ui)
kconfig_add_kcfg_files(ktorrent_logviewer logviewerpluginsettings.kcfgc)

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

