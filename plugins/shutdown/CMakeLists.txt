include_directories(${CMAKE_BINARY_DIR})

ktorrent_add_plugin(ktorrent_shutdown)

set(ktshutdownplugin_dbus_SRC)

set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt_add_dbus_interface(ktshutdownplugin_dbus_SRC ${screensaver_xml} screensaver_interface)

set(powermanagement_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.PowerManagement.xml)
qt_add_dbus_interface(ktshutdownplugin_dbus_SRC ${powermanagement_xml} powermanagement_interface)

set(plasmalogoutprompt_xml ${KTORRENT_DBUS_XML_DIR}/org.kde.LogoutPrompt.xml)
qt_add_dbus_interface(ktshutdownplugin_dbus_SRC ${plasmalogoutprompt_xml} plasmalogoutprompt_interface)

target_sources(ktorrent_shutdown PRIVATE
    ${ktshutdownplugin_dbus_SRC}
    shutdownruleset.cpp
    shutdowntorrentmodel.cpp
    shutdowndlg.cpp
    shutdownplugin.cpp
)

ki18n_wrap_ui(ktorrent_shutdown shutdowndlg.ui)

target_link_libraries(
    ktorrent_shutdown
    ktcore
    KF5::Torrent
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    KF5::WidgetsAddons
)

install(FILES ktorrent_shutdownui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )
