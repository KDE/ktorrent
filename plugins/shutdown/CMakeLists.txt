include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(ktshutdownplugin_SRC shutdownruleset.cpp shutdowntorrentmodel.cpp shutdowndlg.cpp shutdownplugin.cpp)

set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt5_add_dbus_interface(ktshutdownplugin_SRC ${screensaver_xml} screensaver_interface)

set(powermanagement_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.PowerManagement.xml)
qt5_add_dbus_interface(ktshutdownplugin_SRC ${powermanagement_xml} powermanagement_interface)

ki18n_wrap_ui(ktshutdownplugin_SRC shutdowndlg.ui)

add_library(ktorrent_shutdown MODULE ${ktshutdownplugin_SRC})
kcoreaddons_desktop_to_json(ktorrent_shutdown ktorrent_shutdown.desktop)
target_link_libraries(
    ktorrent_shutdown
    ktcore
    KF5::Torrent
    PW::KWorkspace
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    KF5::WidgetsAddons
)

install(TARGETS ktorrent_shutdown  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_shutdownui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
