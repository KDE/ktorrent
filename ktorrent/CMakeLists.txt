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
	viewmanager.cpp 
	viewselectionmodel.cpp
	groupfiltermodel.cpp
	prefdialog.cpp 
	advancedpref.cpp
	networkpref.cpp
	proxypref.cpp
	statusbar.cpp 
	groupview.cpp
	scandlg.cpp
	trayicon.cpp
	pastedialog.cpp
	ipfilterlist.cpp
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
	grouppolicydlg.cpp
	torrentmigratordlg.cpp
	recommendedsettingsdlg.cpp
)
	

kde4_add_ui_files(ktorrent_SRC 
	fileselectdlg.ui 
	qmpref.ui 
	btpref.ui
	scandlg.ui 
	generalpref.ui 
	advancedpref.ui 
	networkpref.ui
	proxypref.ui
	pastedlgbase.ui 
	ipfilterwidget.ui 
	speedlimitsdlg.ui 
	torrentcreatordlg.ui 
	missingfilesdlg.ui
	importdialog.ui
	addpeersdlg.ui
	queuemanagerwidget.ui
	grouppolicydlg.ui
	torrentmigratordlg.ui
	recommendedsettingsdlg.ui
)

kde4_add_app_icon(ktorrent_SRC "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-app-ktorrent.png")

kde4_add_executable(ktorrent ${ktorrent_SRC})

target_link_libraries(ktorrent btcore ktideal ktcore ${KDE4_SOLID_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS})
install(TARGETS ktorrent ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ktorrent.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install(FILES ktorrentplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install(FILES ktorrentui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
install(FILES ktorrent.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

add_subdirectory(icons)
