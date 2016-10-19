set(ktsyndicationplugin_SRC 
	syndicationplugin.cpp 
	syndicationactivity.cpp
	syndicationtab.cpp
	feedlistview.cpp 
	feedlist.cpp 
	feedlistdelegate.cpp
	feedwidget.cpp 
	feedwidgetmodel.cpp
	feedretriever.cpp
	ktfeed.cpp
	linkdownloader.cpp
	managefiltersdlg.cpp
	filter.cpp
	filterlist.cpp
	filterlistview.cpp
	filterlistmodel.cpp
	filtereditor.cpp)

ki18n_wrap_ui(ktsyndicationplugin_SRC feedwidget.ui managefiltersdlg.ui filtereditor.ui)
#kconfig_add_kcfg_files(ktsyndicationplugin_SRC syndicationpluginsettings.kcfgc)
add_library(ktorrent_syndication MODULE ${ktsyndicationplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_syndication ktorrent_syndication.desktop)

target_link_libraries(
    ktorrent_syndication
    ktcore
    KF5::Torrent
    KF5::WebKit
    KF5::Parts
    KF5::Syndication
    KF5::KDELibs4Support
    )
install(TARGETS ktorrent_syndication DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_syndicationui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )

add_subdirectory(icons)
