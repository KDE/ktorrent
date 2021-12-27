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
    KF5::Torrent
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
)
install(FILES ktorrent_downloadorderui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
