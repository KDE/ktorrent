set(ktscanfolderplugin_SRC 
	scanthread.cpp 
	torrentloadqueue.cpp 
	scanfolder.cpp 
	scanfolderplugin.cpp 
	scanfolderprefpage.cpp)

kde4_add_ui_files(ktscanfolderplugin_SRC scanfolderprefpage.ui)
kde4_add_kcfg_files(ktscanfolderplugin_SRC scanfolderpluginsettings.kcfgc)
kde4_add_plugin(ktscanfolderplugin ${ktscanfolderplugin_SRC})

target_link_libraries(
    ktscanfolderplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    ${KDE4_KPARTS_LIBS}
    )
install(TARGETS ktscanfolderplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktscanfolderplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

