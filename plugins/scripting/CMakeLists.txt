add_subdirectory(scripts)

set(ktscriptingplugin_SRC 
	api/scriptingmodule.cpp
	scriptingplugin.cpp 
	scriptmanager.cpp 
	scriptmodel.cpp
	script.cpp)

kde4_add_ui_files(ktscriptingplugin_SRC scriptproperties.ui)
#kde4_add_kcfg_files(ktscriptingplugin_SRC scriptingpluginsettings.kcfgc)
kde4_add_plugin(ktscriptingplugin ${ktscriptingplugin_SRC})

target_link_libraries(
    ktscriptingplugin
    ktcore
    btcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KROSSCORE_LIBS}
    ${KDE4_KROSSUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QT3SUPPORT_LIBRARY}
    )
install(TARGETS ktscriptingplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktscriptingplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
#install(FILES ktscriptingpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
