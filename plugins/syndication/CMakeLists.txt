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

ki18n_wrap_ui(ktsyndicationplugin_SRC feedwidget.ui managefiltersdlg.ui filtereditor.ui)
#kconfig_add_kcfg_files(ktsyndicationplugin_SRC syndicationpluginsettings.kcfgc)
add_library(ktsyndicationplugin MODULE ${ktsyndicationplugin_SRC})

target_link_libraries(
    ktsyndicationplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    Qt5::WebKitWidgets
    KF5::Parts
	${KDE4_SYNDICATION_LIBS}
	${KDE4_KDEWEBKIT_LIBS})
    
install(TARGETS ktsyndicationplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktsyndicationplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktsyndicationpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

add_subdirectory(icons)
