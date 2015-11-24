set(ktsearchplugin_SRC 
	webview.cpp 
	localfilenetworkreply.cpp 
	buffernetworkreply.cpp 
	searchenginelist.cpp 
	searchprefpage.cpp 
	searchwidget.cpp 
	searchtoolbar.cpp 
	searchplugin.cpp 
	searchengine.cpp 
	opensearchdownloadjob.cpp
	searchactivity.cpp)

ki18n_wrap_ui(ktsearchplugin_SRC searchpref.ui)
kconfig_add_kcfg_files(ktsearchplugin_SRC searchpluginsettings.kcfgc)
add_library(ktsearchplugin MODULE ${ktsearchplugin_SRC})

target_link_libraries(
    ktsearchplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    Qt5::WebKitWidgets
    KF5::Parts
    ${KDE4_KDEWEBKIT_LIBS}
    )
install(TARGETS ktsearchplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktsearchplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktsearchpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

add_subdirectory(opensearch)
add_subdirectory(home)
