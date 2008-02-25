include_directories(${CMAKE_CURRENT_BINARY_DIR})

#  set(kt4_SRC app.cpp mainwindow.cpp core.cpp view.cpp viewmodel.cpp main.cpp)
set(ktorrent_SRC 
	main.cpp 
	fileselectdlg.cpp 
	core.cpp 
	gui.cpp 
	app.cpp 
	view.cpp 
	viewmodel.cpp
	viewmenu.cpp
	viewmanager.cpp 
	prefdialog.cpp 
	advancedpref.cpp
	networkpref.cpp
	statusbar.cpp 
	groupview.cpp
	scandlg.cpp
	trayicon.cpp
	pastedialog.cpp
	dbus.cpp
	ipfilterwidget.cpp
	speedlimitsdlg.cpp
	speedlimitsmodel.cpp
	spinboxdelegate.cpp
	torrentcreatordlg.cpp
	missingfilesdlg.cpp
	importdialog.cpp
	addpeersdlg.cpp
	queuemanagerwidget.cpp
	queuemanagermodel.cpp
)
	

kde4_add_ui_files(ktorrent_SRC 
	fileselectdlg.ui 
	qmpref.ui 
	btpref.ui
	scandlg.ui 
	generalpref.ui 
	advancedpref.ui 
	networkpref.ui
	pastedlgbase.ui 
	ipfilterwidget.ui 
	speedlimitsdlg.ui 
	torrentcreatordlg.ui 
	missingfilesdlg.ui
	importdialog.ui
	addpeersdlg.ui
	queuemanagerwidget.ui
)

kde4_add_executable(ktorrent ${ktorrent_SRC})

target_link_libraries(ktorrent ktideal ktcore ${KDE4_KDEUI_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS})
install(TARGETS ktorrent DESTINATION ${BIN_INSTALL_DIR})
install(FILES ktorrent.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install(FILES ktorrentplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install(FILES ktorrentui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
install(FILES ktorrent.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

add_subdirectory(icons)
