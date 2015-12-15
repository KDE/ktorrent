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
add_library(ktorrent_search MODULE ${ktsearchplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_search ktorrent_search.desktop)

target_link_libraries(
    ktorrent_search
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    Qt5::WebKitWidgets
    KF5::Parts
    KF5::WebKit
    KF5::IconThemes
    KF5::Notifications
    )
install(TARGETS ktorrent_search  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_searchui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )

add_subdirectory(opensearch)
add_subdirectory(home)
