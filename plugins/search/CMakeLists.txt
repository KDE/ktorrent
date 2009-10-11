set(ktsearchplugin_SRC 
	homepage.cpp 
	searchenginelist.cpp 
	htmlpart.cpp 
	searchprefpage.cpp 
	searchwidget.cpp 
	searchtoolbar.cpp 
	searchplugin.cpp 
	searchengine.cpp 
	opensearchdownloadjob.cpp
	searchactivity.cpp)

kde4_add_ui_files(ktsearchplugin_SRC searchpref.ui)
kde4_add_kcfg_files(ktsearchplugin_SRC searchpluginsettings.kcfgc)
kde4_add_plugin(ktsearchplugin ${ktsearchplugin_SRC})

target_link_libraries(
    ktsearchplugin
    ktcore
    btcore
    ${KDE4_KHTML_LIBS}
    ${QT_QT3SUPPORT_LIBRARY}
    )
install(TARGETS ktsearchplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktsearchplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktsearchpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

add_subdirectory(opensearch)
add_subdirectory(home)