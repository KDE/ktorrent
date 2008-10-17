find_package(KdepimLibs REQUIRED)
find_package(Boost REQUIRED)

set(ktsyndicationplugin_SRC 
	syndicationplugin.cpp 
	feedlistview.cpp 
	feedlist.cpp 
	feed.cpp 
	feedwidget.cpp 
	feedwidgetmodel.cpp
	linkdownloader.cpp)

kde4_add_ui_files(ktsyndicationplugin_SRC feedwidget.ui)
#kde4_add_kcfg_files(ktsyndicationplugin_SRC syndicationpluginsettings.kcfgc)
kde4_add_plugin(ktsyndicationplugin ${ktsyndicationplugin_SRC})

target_link_libraries(
    ktsyndicationplugin
    ktcore
    btcore
    ${KDE4_KIO_LIBS}
	${KDE4_SYNDICATION_LIBS}
	${QT_QTWEBKIT_LIBRARY})
    
install(TARGETS ktsyndicationplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktsyndicationplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktsyndicationpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )

