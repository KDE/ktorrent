set(ktmagnetgeneratorplugin_SRC 
	magnetgeneratorprefwidget.cpp
	magnetgeneratorplugin.cpp)

kde4_add_ui_files(ktmagnetgeneratorplugin_SRC magnetgeneratorprefwidget.ui)
kde4_add_kcfg_files(ktmagnetgeneratorplugin_SRC magnetgeneratorpluginsettings.kcfgc)
kde4_add_plugin(ktmagnetgeneratorplugin ${ktmagnetgeneratorplugin_SRC})

target_link_libraries(
    ktmagnetgeneratorplugin
    ktcore
    btcore
    ${KDE4_KPARTS_LIBS}
	${KDE4_KDEUI_LIBS}
    ${QT_QT3SUPPORT_LIBRARY}
    )
install(TARGETS ktmagnetgeneratorplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktmagnetgeneratorplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktmagnetgeneratorpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
