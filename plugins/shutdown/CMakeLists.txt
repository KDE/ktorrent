include_directories(${CMAKE_BINARY_DIR})

kcoreaddons_add_plugin(ktorrent_shutdown INSTALL_NAMESPACE "ktorrent")

set(ktshutdownplugin_dbus_SRC)

set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt5_add_dbus_interface(ktshutdownplugin_dbus_SRC ${screensaver_xml} screensaver_interface)

set(powermanagement_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.PowerManagement.xml)
qt5_add_dbus_interface(ktshutdownplugin_dbus_SRC ${powermanagement_xml} powermanagement_interface)

target_sources(ktorrent_shutdown PRIVATE
    ${ktshutdownplugin_dbus_SRC}
    shutdownruleset.cpp
    shutdowntorrentmodel.cpp
    shutdowndlg.cpp
    shutdownplugin.cpp
)

ki18n_wrap_ui(ktorrent_shutdown shutdowndlg.ui)

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

install(FILES ktorrent_shutdownui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
