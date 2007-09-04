set(ktwebinterfaceplugin_SRC 
	httpresponseheader.cpp 
	phpcodegenerator.cpp 
	phphandler.cpp 
	phpcommandhandler.cpp 
	httpclienthandler.cpp 
	httpserver.cpp
	webinterfaceprefwidget.cpp
	webinterfaceplugin.cpp)

kde4_add_ui_files(ktwebinterfaceplugin_SRC webinterfaceprefwidget.ui)
kde4_add_kcfg_files(ktwebinterfaceplugin_SRC webinterfacepluginsettings.kcfgc)
kde4_add_plugin(ktwebinterfaceplugin ${ktwebinterfaceplugin_SRC})

target_link_libraries(ktwebinterfaceplugin ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} -lkparts ktcore )
install(TARGETS ktwebinterfaceplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktwebinterfaceplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
add_subdirectory(www)
