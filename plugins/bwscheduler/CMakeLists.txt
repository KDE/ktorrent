ktorrent_add_plugin(BandwidthSchedulerPlugin)

set(ktbwschedulerplugin_dbus_SRC)
set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt_add_dbus_interface(ktbwschedulerplugin_dbus_SRC ${screensaver_xml} screensaver_interface)

target_sources(BandwidthSchedulerPlugin PRIVATE
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

ki18n_wrap_ui(BandwidthSchedulerPlugin edititemdlg.ui bwprefpage.ui)
kconfig_add_kcfg_files(BandwidthSchedulerPlugin bwschedulerpluginsettings.kcfgc)

target_link_libraries(
    BandwidthSchedulerPlugin
    ktcore
    Boost::boost
    KTorrent6
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui
    KF6::WidgetsAddons
)
install(FILES ktorrent_bwschedulerui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )

