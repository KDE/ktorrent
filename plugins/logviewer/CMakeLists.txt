ktorrent_add_plugin(LogViewerPlugin)

target_sources(LogViewerPlugin PRIVATE
    logviewerplugin.cpp
    logflags.cpp
    logviewer.cpp
    logprefpage.cpp
    logflagsdelegate.cpp
)

ki18n_wrap_ui(LogViewerPlugin logprefwidget.ui)
kconfig_add_kcfg_files(LogViewerPlugin logviewerpluginsettings.kcfgc)

target_link_libraries(
    LogViewerPlugin
    ktcore
    KTorrent6
    KF6::ConfigCore
    KF6::I18n
    KF6::XmlGui
)
