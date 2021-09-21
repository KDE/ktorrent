kcoreaddons_add_plugin(ktorrent_bwscheduler  INSTALL_NAMESPACE "ktorrent")

set(ktbwschedulerplugin_dbus_SRC)
set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt5_add_dbus_interface(ktbwschedulerplugin_dbus_SRC ${screensaver_xml} screensaver_interface)

target_sources(ktorrent_bwscheduler PRIVATE
	${ktbwschedulerplugin_dbus_SRC}
	bwschedulerplugin.cpp 
	weekview.cpp 
	weekscene.cpp 
	schedule.cpp 
	scheduleeditor.cpp 
	schedulegraphicsitem.cpp 
	bwprefpage.cpp 
	guidanceline.cpp 
	edititemdlg.cpp 
	weekdaymodel.cpp
)

ki18n_wrap_ui(ktorrent_bwscheduler edititemdlg.ui bwprefpage.ui)
kconfig_add_kcfg_files(ktorrent_bwscheduler bwschedulerpluginsettings.kcfgc)

kcoreaddons_desktop_to_json(ktorrent_bwscheduler ktorrent_bwscheduler.desktop)

target_link_libraries(
    ktorrent_bwscheduler
    ktcore
    Boost::boost
    KF5::Torrent
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    KF5::WidgetsAddons
)
install(FILES ktorrent_bwschedulerui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )

