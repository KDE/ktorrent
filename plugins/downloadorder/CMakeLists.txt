ktorrent_add_plugin(ktorrent_downloadorder)

target_sources(ktorrent_downloadorder PRIVATE
    downloadorderplugin.cpp
    downloadorderdialog.cpp
    downloadordermanager.cpp
    downloadordermodel.cpp
)

ki18n_wrap_ui(ktorrent_downloadorder downloadorderwidget.ui)

target_link_libraries(
    ktorrent_downloadorder
    ktcore
    KTorrent6
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui
)
install(FILES ktorrent_downloadorderui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )
