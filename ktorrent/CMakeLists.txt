include_directories(${CMAKE_CURRENT_BINARY_DIR})

#  set(kt4_SRC app.cpp mainwindow.cpp core.cpp view.cpp viewmodel.cpp main.cpp)
set(ktorrent_SRC 
	main.cpp 
	fileselectdlg.cpp 
	core.cpp 
	gui.cpp 
	app.cpp 
	torrentactivity.cpp
	groupfiltermodel.cpp
	statusbar.cpp 
	groupview.cpp
	scandlg.cpp
	
	scanlistener.cpp
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
	
	pref/prefdialog.cpp 
	pref/advancedpref.cpp
	pref/networkpref.cpp
	pref/proxypref.cpp
	pref/qmpref.cpp
	pref/generalpref.cpp
	pref/recommendedsettingsdlg.cpp
	pref/btpref.cpp
	
	view/view.cpp 
	view/viewmodel.cpp
	view/viewmanager.cpp 
	view/viewdelegate.cpp
	view/viewselectionmodel.cpp
	view/scanextender.cpp
)
	

kde4_add_ui_files(ktorrent_SRC 
	fileselectdlg.ui 
	scandlg.ui 
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
	
	pref/qmpref.ui 
	pref/btpref.ui
	pref/generalpref.ui 
	pref/advancedpref.ui 
	pref/networkpref.ui
	pref/proxypref.ui
	pref/recommendedsettingsdlg.ui
	
	view/scanextender.ui
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
