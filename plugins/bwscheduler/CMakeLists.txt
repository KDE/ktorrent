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
add_library(ktbwschedulerplugin MODULE ${ktbwschedulerplugin_SRC})

target_link_libraries(
    ktbwschedulerplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    KF5::Parts
    )
install(TARGETS ktbwschedulerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktbwschedulerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktbwschedulerpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

