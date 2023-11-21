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
    Qt6::WebEngineWidgets
    KTorrent6
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Syndication
    KF6::WidgetsAddons
    KF6::XmlGui
)
install(FILES ktorrent_syndicationui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )

add_subdirectory(icons)
