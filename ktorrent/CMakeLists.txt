add_executable(ktorrent_app)
set_target_properties(ktorrent_app  PROPERTIES
    OUTPUT_NAME ktorrent
    CXX_STANDARD 14
)

set(ktorrent_dbus_SRC)
set(powermanagementinhibit_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.PowerManagement.Inhibit.xml)
qt_add_dbus_interface(ktorrent_dbus_SRC ${powermanagementinhibit_xml} powermanagementinhibit_interface)

target_sources(ktorrent_app PRIVATE
	${ktorrent_dbus_SRC}

	main.cpp
	core.cpp
	gui.cpp
	torrentactivity.cpp
	statusbar.cpp
	trayicon.cpp
	ipfilterlist.cpp
	ipfilterwidget.cpp
	statusbarofflineindicator.cpp
	
	tools/queuemanagerwidget.cpp
	tools/queuemanagermodel.cpp
	tools/magnetmodel.cpp
	tools/magnetview.cpp
	
	groups/grouppolicydlg.cpp
	groups/groupfiltermodel.cpp
	groups/groupview.cpp
	groups/groupswitcher.cpp
	
	dialogs/pastedialog.cpp
	dialogs/speedlimitsdlg.cpp
	dialogs/speedlimitsmodel.cpp
	dialogs/spinboxdelegate.cpp
	dialogs/torrentcreatordlg.cpp
	dialogs/missingfilesdlg.cpp
	dialogs/importdialog.cpp
	dialogs/addpeersdlg.cpp
	dialogs/fileselectdlg.cpp 
	
	pref/prefdialog.cpp 
	pref/colorpref.cpp
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

ki18n_wrap_ui(ktorrent_app
	ipfilterwidget.ui 
	
	dialogs/speedlimitsdlg.ui 
	dialogs/pastedlgbase.ui 
	dialogs/torrentcreatordlg.ui 
	dialogs/missingfilesdlg.ui
	dialogs/importdialog.ui
	dialogs/addpeersdlg.ui
	dialogs/fileselectdlg.ui 
	
	groups/grouppolicydlg.ui
	
	pref/qmpref.ui 
	pref/btpref.ui
	pref/generalpref.ui 
	pref/colorpref.ui
	pref/advancedpref.ui 
	pref/networkpref.ui
	pref/proxypref.ui
	pref/recommendedsettingsdlg.ui
	
	view/scanextender.ui
	view/propertiesdlg.ui
)

# collect icons
set(KTORRENT_ICONS_PNG
    icons/16-apps-ktorrent.png
    icons/22-apps-ktorrent.png
    icons/32-apps-ktorrent.png
    icons/48-apps-ktorrent.png
    icons/64-apps-ktorrent.png
    icons/128-apps-ktorrent.png
)

ecm_add_app_icon(ktorrent_app ICONS ${KTORRENT_ICONS_PNG})

target_link_libraries(ktorrent_app
    ktcore
    KTorrent6
    KF6::Crash
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::DBusAddons
    KF6::GlobalAccel
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::Notifications
    KF6::NotifyConfig
    KF6::Parts
    KF6::Solid
    KF6::StatusNotifierItem
    KF6::WidgetsAddons
    KF6::WindowSystem
    KF6::XmlGui
    KF6::Crash
)

install(TARGETS ktorrent_app ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.ktorrent.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install(FILES ktorrentui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )
install(FILES kttorrentactivityui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )
install(FILES ktorrent.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR} )
install(FILES org.kde.ktorrent.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

add_subdirectory(icons)
