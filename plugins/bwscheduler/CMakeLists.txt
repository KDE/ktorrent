set(ktbwschedulerplugin_SRC 
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

set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
qt5_add_dbus_interface(ktbwschedulerplugin_SRC ${screensaver_xml} screensaver_interface)

ki18n_wrap_ui(ktbwschedulerplugin_SRC edititemdlg.ui bwprefpage.ui)
kconfig_add_kcfg_files(ktbwschedulerplugin_SRC bwschedulerpluginsettings.kcfgc)
add_library(ktorrent_bwscheduler MODULE ${ktbwschedulerplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_bwscheduler ktorrent_bwscheduler.desktop)

target_link_libraries(
    ktorrent_bwscheduler
    ktcore
    KF5::Torrent
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    KF5::WidgetsAddons
)
install(TARGETS ktorrent_bwscheduler  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_bwschedulerui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )

