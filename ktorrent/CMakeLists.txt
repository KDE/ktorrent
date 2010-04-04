include_directories(${CMAKE_CURRENT_BINARY_DIR})

#  set(kt4_SRC app.cpp mainwindow.cpp core.cpp view.cpp viewmodel.cpp main.cpp)
set(ktorrent_SRC 
	main.cpp 
	core.cpp 
	gui.cpp 
	app.cpp 
	torrentactivity.cpp
	statusbar.cpp 
	scanlistener.cpp
	trayicon.cpp
	ipfilterlist.cpp
	ipfilterwidget.cpp
	
	tools/queuemanagerwidget.cpp
	tools/queuemanagermodel.cpp
	tools/magnetmodel.cpp
	tools/magnetview.cpp
	
	groups/grouppolicydlg.cpp
	groups/groupfiltermodel.cpp
	groups/groupview.cpp
	
	dialogs/pastedialog.cpp
	dialogs/speedlimitsdlg.cpp
	dialogs/speedlimitsmodel.cpp
	dialogs/spinboxdelegate.cpp
	dialogs/torrentcreatordlg.cpp
	dialogs/missingfilesdlg.cpp
	dialogs/importdialog.cpp
	dialogs/addpeersdlg.cpp
	dialogs/torrentmigratordlg.cpp
	dialogs/fileselectdlg.cpp 
	
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
	ipfilterwidget.ui 
	
	dialogs/speedlimitsdlg.ui 
	dialogs/pastedlgbase.ui 
	dialogs/torrentcreatordlg.ui 
	dialogs/missingfilesdlg.ui
	dialogs/importdialog.ui
	dialogs/addpeersdlg.ui
	dialogs/torrentmigratordlg.ui
	dialogs/fileselectdlg.ui 
	
	groups/grouppolicydlg.ui
	
	tools/queuemanagerwidget.ui
	
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

target_link_libraries(ktorrent ${LIBKTORRENT_LIBRARIES} ktideal ktcore ${KDE4_SOLID_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS})
install(TARGETS ktorrent ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ktorrent.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install(FILES ktorrentplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install(FILES magnet.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktorrentui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
install(FILES ktorrent.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

add_subdirectory(icons)
