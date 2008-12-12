set(ktwebinterfaceplugin_SRC 
	httpresponseheader.cpp 
	httpclienthandler.cpp 
	httpserver.cpp
	webinterfaceprefwidget.cpp
	webinterfaceplugin.cpp
	webcontentgenerator.cpp
	globaldatagenerator.cpp
	torrentlistgenerator.cpp
	torrentfilesgenerator.cpp
	challengegenerator.cpp
	settingsgenerator.cpp
	loginhandler.cpp
	logouthandler.cpp
	actionhandler.cpp
	iconhandler.cpp
	torrentposthandler.cpp)

kde4_add_ui_files(ktwebinterfaceplugin_SRC webinterfaceprefwidget.ui)
kde4_add_kcfg_files(ktwebinterfaceplugin_SRC webinterfacepluginsettings.kcfgc)
kde4_add_plugin(ktwebinterfaceplugin ${ktwebinterfaceplugin_SRC})

target_link_libraries(
    ktwebinterfaceplugin
    ktcore
    btcore
    ${KDE4_KPARTS_LIBS}
    ${QT_QT3SUPPORT_LIBRARY}
    )
install(TARGETS ktwebinterfaceplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktwebinterfaceplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
add_subdirectory(www)
