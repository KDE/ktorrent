set(ktbitfinderplugin_SRC bitfinderplugin.cpp 
				source/sourcesview.cpp
				filter/capture.cpp
				filter/capturechecker.cpp
				filter/capturecheckerdetails.cpp
				filter/filter.cpp
				filter/filterlistmodel.cpp
				filter/filtersview.cpp
				filter/filterdetails.cpp
				)

kde4_add_ui_files(ktbitfinderplugin_SRC
			filter/capturecheckerdetails.ui
			filter/filterdetails.ui)

kde4_add_plugin(ktbitfinderplugin ${ktbitfinderplugin_SRC})

target_link_libraries(ktbitfinderplugin ktcore)
install(TARGETS ktbitfinderplugin DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktbitfinderplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
