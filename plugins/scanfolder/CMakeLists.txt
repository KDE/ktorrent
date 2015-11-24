set(ktscanfolderplugin_SRC 
	scanthread.cpp 
	torrentloadqueue.cpp 
	scanfolder.cpp 
	scanfolderplugin.cpp 
	scanfolderprefpage.cpp)

ki18n_wrap_ui(ktscanfolderplugin_SRC scanfolderprefpage.ui)
kconfig_add_kcfg_files(ktscanfolderplugin_SRC scanfolderpluginsettings.kcfgc)
add_library(ktscanfolderplugin MODULE ${ktscanfolderplugin_SRC})

target_link_libraries(
    ktscanfolderplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    KF5::Parts
    )
install(TARGETS ktscanfolderplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktscanfolderplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

