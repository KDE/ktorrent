set(kthtmlinterfaceplugin_SRC htmlinterfaceplugin.cpp mongoose.c webserver.cpp torrentlistgenerator.cpp)

ki18n_wrap_ui(kthtmlinterfaceplugin_SRC htmlinterfacewidget.ui)

add_library(ktorrent_htmlinterface MODULE ${kthtmlinterfaceplugin_SRC})

add_subdirectory(html)

kcoreaddons_desktop_to_json(ktorrent_htmlinterface  ktorrent_htmlinterface.desktop)

target_link_libraries(
    ktorrent_htmlinterface
    ktcore
    KF5::Torrent
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
)

install(TARGETS ktorrent_htmlinterface  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_htmlinterface.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
