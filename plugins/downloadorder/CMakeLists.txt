ktorrent_add_plugin(DownloadOrderPlugin)

target_sources(DownloadOrderPlugin PRIVATE
    downloadorderplugin.cpp
    downloadorderdialog.cpp
    downloadordermanager.cpp
    downloadordermodel.cpp
)

ki18n_wrap_ui(DownloadOrderPlugin downloadorderwidget.ui)

target_link_libraries(
    DownloadOrderPlugin
    ktcore
    KTorrent6
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui
)
install(FILES ktorrent_downloadorderui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )
