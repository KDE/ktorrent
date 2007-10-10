include_directories(${CMAKE_CURRENT_BINARY_DIR})

#  set(kt4_SRC app.cpp mainwindow.cpp core.cpp view.cpp viewmodel.cpp main.cpp)
set(ktorrent_SRC 
	main.cpp 
	fileselectdlg.cpp 
	core.cpp 
	gui.cpp 
	app.cpp 
	view.cpp 
	viewitem.cpp 
	viewmenu.cpp
	viewmanager.cpp 
	prefdialog.cpp 
	advancedpref.cpp
	statusbar.cpp 
	groupview.cpp
	scandlg.cpp
	trayicon.cpp
	pastedialog.cpp
	dbus.cpp
	ipfilterwidget.cpp
	speedlimitsdlg.cpp
	torrentcreatordlg.cpp
	missingfilesdlg.cpp
	importdialog.cpp
	addpeersdlg.cpp
)
	

kde4_add_ui_files(ktorrent_SRC 
	fileselectdlg.ui 
	globalpref.ui 
	scandlg.ui 
	generalpref.ui 
	advancedpref.ui 
	pastedlgbase.ui 
	ipfilterwidget.ui 
	speedlimitsdlg.ui 
	torrentcreatordlg.ui 
	missingfilesdlg.ui
	importdialog.ui
	addpeersdlg.ui
)

kde4_add_executable(ktorrent ${ktorrent_SRC})

target_link_libraries(ktorrent ktideal ktcore ${KDE4_KDEUI_LIBS})
install(TARGETS ktorrent DESTINATION ${BIN_INSTALL_DIR})
install(FILES ktorrent.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install(FILES ktorrentplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install(FILES ktorrentui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

add_subdirectory(icons)
