find_package(Taglib REQUIRED)
include_directories(${TAGLIB_INCLUDES})
#find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Multimedia MultimediaWidgets)
#include_directories(${Qt5Multimedia_INCLUDE_DIRS})
include_directories(${PHONON_INCLUDES})

set(ktmediaplayerplugin_SRC
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

set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt5_add_dbus_interface(ktmediaplayerplugin_SRC ${screensaver_xml} screensaver_interface)

set(powermanagementinhibit_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.PowerManagement.Inhibit.xml)
qt5_add_dbus_interface(ktmediaplayerplugin_SRC ${powermanagementinhibit_xml} powermanagementinhibit_interface)

ki18n_wrap_ui(ktmediaplayerplugin_SRC mediacontroller.ui)
kconfig_add_kcfg_files(ktmediaplayerplugin_SRC mediaplayerpluginsettings.kcfgc)
add_library(ktorrent_mediaplayer MODULE ${ktmediaplayerplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_mediaplayer ktorrent_mediaplayer.desktop)

target_link_libraries(
    ktorrent_mediaplayer
    ktcore
    ${TAGLIB_LIBRARIES}
    Phonon::phonon4qt5
    KF5::Torrent
    KF5::I18n
    KF5::KIOFileWidgets
    KF5::XmlGui
    KF5::WidgetsAddons
#    Qt5::MultimediaWidgets
    )
install(TARGETS ktorrent_mediaplayer DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_mediaplayerui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
