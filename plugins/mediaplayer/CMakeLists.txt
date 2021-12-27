find_package(Taglib REQUIRED)
#find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Multimedia MultimediaWidgets)
#include_directories(${Qt5Multimedia_INCLUDE_DIRS})
include_directories(SYSTEM ${PHONON_INCLUDES})

ktorrent_add_plugin(ktorrent_mediaplayer)

set(ktmediaplayerplugin_dbus_SRC)
set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt5_add_dbus_interface(ktmediaplayerplugin_dbus_SRC ${screensaver_xml} screensaver_interface)

set(powermanagementinhibit_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.PowerManagement.Inhibit.xml)
qt5_add_dbus_interface(ktmediaplayerplugin_dbus_SRC ${powermanagementinhibit_xml} powermanagementinhibit_interface)

target_sources(ktorrent_mediaplayer PRIVATE
	${ktmediaplayerplugin_dbus_SRC}
	mediacontroller.cpp
	playlist.cpp
	playlistwidget.cpp
	mediaplayeractivity.cpp
	mediaplayerplugin.cpp
	mediaview.cpp
	mediamodel.cpp
	mediafile.cpp
	mediafilestream.cpp
	mediaplayer.cpp
	videowidget.cpp
	videochunkbar.cpp
)

ki18n_wrap_ui(ktorrent_mediaplayer mediacontroller.ui)
kconfig_add_kcfg_files(ktorrent_mediaplayer mediaplayerpluginsettings.kcfgc)

target_link_libraries(
    ktorrent_mediaplayer
    ktcore
    Taglib::Taglib
    Phonon::phonon4qt5
    KF5::Torrent
    KF5::I18n
    KF5::KIOFileWidgets
    KF5::XmlGui
    KF5::WidgetsAddons
#    Qt5::MultimediaWidgets
    )
install(FILES ktorrent_mediaplayerui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
