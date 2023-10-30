ktorrent_add_plugin(ktorrent_logviewer)

target_sources(ktorrent_logviewer PRIVATE
    logviewerplugin.cpp
    logflags.cpp
    logviewer.cpp
    logprefpage.cpp
    logflagsdelegate.cpp
)

ki18n_wrap_ui(ktorrent_logviewer logprefwidget.ui)
kconfig_add_kcfg_files(ktorrent_logviewer logviewerpluginsettings.kcfgc)

target_link_libraries(
    ktorrent_logviewer
    ktcore
    KF5::Torrent
    KF6::ConfigCore
    KF6::I18n
    KF6::XmlGui
)
