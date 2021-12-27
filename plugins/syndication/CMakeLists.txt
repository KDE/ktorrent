ktorrent_add_plugin(ktorrent_syndication)

target_sources(ktorrent_syndication PRIVATE
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

ki18n_wrap_ui(ktorrent_syndication feedwidget.ui managefiltersdlg.ui filtereditor.ui)

target_link_libraries(
    ktorrent_syndication
    ktcore
    Qt5::WebEngineWidgets
    KF5::Torrent
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Syndication
    KF5::WidgetsAddons
    KF5::XmlGui
)
install(FILES ktorrent_syndicationui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )

add_subdirectory(icons)
