set(ktbitfinderplugin_SRC bitfinderplugin.cpp)

#kde4_add_ui_files(ktbitfinderplugin_SRC bitfinderpref.ui)
kde4_add_plugin(ktbitfinderplugin ${ktbitfinderplugin_SRC})

target_link_libraries(ktbitfinderplugin ktcore)
install(TARGETS ktbitfinderplugin DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktbitfinderplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
