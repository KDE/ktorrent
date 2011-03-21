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
QT4_ADD_DBUS_INTERFACE(ktbwschedulerplugin_SRC ${screensaver_xml} screensaver_interface)

kde4_add_ui_files(ktbwschedulerplugin_SRC edititemdlg.ui bwprefpage.ui)
kde4_add_kcfg_files(ktbwschedulerplugin_SRC bwschedulerpluginsettings.kcfgc)
kde4_add_plugin(ktbwschedulerplugin ${ktbwschedulerplugin_SRC})

target_link_libraries(
    ktbwschedulerplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS} 
    ${KDE4_SOLID_LIBS}
    )
install(TARGETS ktbwschedulerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktbwschedulerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktbwschedulerpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

