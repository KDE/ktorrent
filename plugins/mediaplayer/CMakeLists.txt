find_package(Taglib REQUIRED)
#find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Multimedia MultimediaWidgets)
#include_directories(${Qt5Multimedia_INCLUDE_DIRS})
include_directories(SYSTEM ${PHONON_INCLUDE_DIR})

ktorrent_add_plugin(MediaPlayerPlugin)

set(ktmediaplayerplugin_dbus_SRC)
set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt_add_dbus_interface(ktmediaplayerplugin_dbus_SRC ${screensaver_xml} screensaver_interface)

set(powermanagementinhibit_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.PowerManagement.Inhibit.xml)
qt_add_dbus_interface(ktmediaplayerplugin_dbus_SRC ${powermanagementinhibit_xml} powermanagementinhibit_interface)

target_sources(MediaPlayerPlugin PRIVATE
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

ki18n_wrap_ui(MediaPlayerPlugin mediacontroller.ui)
kconfig_add_kcfg_files(MediaPlayerPlugin mediaplayerpluginsettings.kcfgc)

target_link_libraries(
    MediaPlayerPlugin
    ktcore
    Taglib::Taglib
    Phonon::phonon4qt6
    KTorrent6
    KF6::I18n
    KF6::KIOFileWidgets
    KF6::XmlGui
    KF6::WidgetsAddons
#    Qt:::MultimediaWidgets
    )
install(FILES ktorrent_mediaplayerui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )
