ktorrent_add_plugin(ktorrent_search)

target_sources(ktorrent_search PRIVATE
	webview.cpp 
	searchenginelist.cpp 
	searchprefpage.cpp 
	searchwidget.cpp 
	searchtoolbar.cpp 
	searchplugin.cpp 
	searchengine.cpp 
	opensearchdownloadjob.cpp
        searchactivity.cpp
        proxy_helper.cpp
        magneturlschemehandler.cpp)

ki18n_wrap_ui(ktorrent_search searchpref.ui)
kconfig_add_kcfg_files(ktorrent_search searchpluginsettings.kcfgc)

target_link_libraries(
    ktorrent_search
    ktcore
    Qt6::WebEngineWidgets
    KTorrent6
    KF6::CoreAddons
    KF6::Completion
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Notifications
    KF6::WidgetsAddons
    KF6::XmlGui
)

install(FILES ktorrent_searchui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )

add_subdirectory(opensearch)
add_subdirectory(home)
