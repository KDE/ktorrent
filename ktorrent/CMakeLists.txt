include_directories(${CMAKE_CURRENT_BINARY_DIR})

#  set(kt4_SRC app.cpp mainwindow.cpp core.cpp view.cpp viewmodel.cpp main.cpp)
set(ktorrent_SRC
	main.cpp 
	core.cpp 
	gui.cpp 
	app.cpp 
	torrentactivity.cpp
	statusbar.cpp 
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
	groups/groupmodel.cpp 
	groups/groupswitcher.cpp
	
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
	view/viewdelegate.cpp
	view/viewselectionmodel.cpp
	view/viewjobtracker.cpp 
	view/scanextender.cpp
	view/propertiesdlg.cpp
	view/torrentsearchbar.cpp 
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
	
	pref/qmpref.ui 
	pref/btpref.ui
	pref/generalpref.ui 
	pref/advancedpref.ui 
	pref/networkpref.ui
	pref/proxypref.ui
	pref/recommendedsettingsdlg.ui
	
	view/scanextender.ui
	view/propertiesdlg.ui
)

kde4_add_app_icon(ktorrent_SRC "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-app-ktorrent.png")

kde4_add_executable(ktorrent_app ${ktorrent_SRC})
set_target_properties(ktorrent_app  PROPERTIES OUTPUT_NAME ktorrent)

target_link_libraries(ktorrent_app ${LIBKTORRENT_LIBRARIES} ktcore ${KDE4_SOLID_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS})
install(TARGETS ktorrent_app ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS ktorrent.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install(FILES ktorrentplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install(FILES ktorrentui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
install(FILES kttorrentactivityui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
install(FILES ktorrent.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

if (NOT ${ENABLE_KIO_MAGNET})
    install(FILES magnet.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
endif (NOT ${ENABLE_KIO_MAGNET})
    

add_subdirectory(icons)
