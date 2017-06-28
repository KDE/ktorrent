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

ki18n_wrap_ui(ktwebinterfaceplugin_SRC webinterfaceprefwidget.ui)
kconfig_add_kcfg_files(ktwebinterfaceplugin_SRC webinterfacepluginsettings.kcfgc)
add_library(ktwebinterfaceplugin MODULE ${ktwebinterfaceplugin_SRC})

target_link_libraries(
    ktwebinterfaceplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
)
install(TARGETS ktwebinterfaceplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktwebinterfaceplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
add_subdirectory(www)
