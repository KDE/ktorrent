add_subdirectory(scripts)

set(ktscriptingplugin_SRC 
	scriptingplugin.cpp 
	scriptmanager.cpp 
	scriptmodel.cpp
	script.cpp)

kde4_add_ui_files(ktscriptingplugin_SRC scriptproperties.ui)
#kde4_add_kcfg_files(ktscriptingplugin_SRC scriptingpluginsettings.kcfgc)
kde4_add_plugin(ktscriptingplugin ${ktscriptingplugin_SRC})

target_link_libraries(ktscriptingplugin ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KROSSUI_LIBS} ktcore )
install(TARGETS ktscriptingplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktscriptingplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
#install(FILES ktscriptingpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
