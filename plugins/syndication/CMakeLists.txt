find_package(KdepimLibs REQUIRED)
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${KDEPIMLIBS_INCLUDE_DIR})

set(ktsyndicationplugin_SRC 
	syndicationplugin.cpp 
	syndicationactivity.cpp
	syndicationtab.cpp
	feedlistview.cpp 
	feedlist.cpp 
	feedlistdelegate.cpp
	feed.cpp 
	feedwidget.cpp 
	feedwidgetmodel.cpp
	feedretriever.cpp
	linkdownloader.cpp
	managefiltersdlg.cpp
	filter.cpp
	filterlist.cpp
	filterlistview.cpp
	filterlistmodel.cpp
	filtereditor.cpp)

kde4_add_ui_files(ktsyndicationplugin_SRC feedwidget.ui managefiltersdlg.ui filtereditor.ui)
#kde4_add_kcfg_files(ktsyndicationplugin_SRC syndicationpluginsettings.kcfgc)
kde4_add_plugin(ktsyndicationplugin ${ktsyndicationplugin_SRC})

target_link_libraries(
    ktsyndicationplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    ${KDE4_KPARTS_LIBS}
	${KDE4_SYNDICATION_LIBS})
    
install(TARGETS ktsyndicationplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktsyndicationplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktsyndicationpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

add_subdirectory(icons)
