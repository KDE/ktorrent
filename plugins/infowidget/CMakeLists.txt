set(ktinfowidgetplugin_SRC 
	infowidgetplugin.cpp 
	iwprefpage.cpp 
	monitor.cpp 
	chunkbar.cpp 
	availabilitychunkbar.cpp 
	downloadedchunkbar.cpp 
	statustab.cpp
	iwfiletreeitem.cpp
	iwfiletreediritem.cpp
	fileview.cpp
	peerview.cpp
	chunkdownloadview.cpp
	trackerview.cpp)

kde4_add_ui_files(ktinfowidgetplugin_SRC iwprefpage.ui statustab.ui chunkdownloadview.ui trackerview.ui)
kde4_add_kcfg_files(ktinfowidgetplugin_SRC infowidgetpluginsettings.kcfgc)
kde4_add_plugin(ktinfowidgetplugin ${ktinfowidgetplugin_SRC})

target_link_libraries(ktinfowidgetplugin ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} -lkparts ktcore )
install(TARGETS ktinfowidgetplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktinfowidgetplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

