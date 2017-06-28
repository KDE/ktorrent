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
        searchactivity.cpp
        proxy_helper.cpp)

ki18n_wrap_ui(ktsearchplugin_SRC searchpref.ui)
kconfig_add_kcfg_files(ktsearchplugin_SRC searchpluginsettings.kcfgc)
add_library(ktorrent_search MODULE ${ktsearchplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_search ktorrent_search.desktop)

target_link_libraries(
    ktorrent_search
    ktcore
    Qt5::WebKitWidgets
    KF5::Torrent
    KF5::CoreAddons
    KF5::Completion
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Notifications
    KF5::WebKit
    KF5::WidgetsAddons
    KF5::XmlGui
)

install(TARGETS ktorrent_search  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_searchui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )

add_subdirectory(opensearch)
add_subdirectory(home)
