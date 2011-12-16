MACRO_ENSURE_VERSION( "4.6.0" "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}" QT_VERSION_OK)
if(NOT QT_VERSION_OK)
	
	message(WARNING "Qt version to old for plasma applet, it will not be built, Qt 4.6.0 is required")

else(NOT QT_VERSION_OK)
	
	
	set(ktapplet_SRCS applet.cpp chunkbar.cpp fadingitem.cpp fadingnavigationwidget.cpp)
	
	kde4_add_ui_files(ktapplet_SRCS appletconfig.ui)
	
	kde4_add_plugin(plasma_applet_ktorrent ${ktapplet_SRCS})
	target_link_libraries(plasma_applet_ktorrent  ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${LIBKTORRENT_LIBRARIES} ktcore)
	
	install(TARGETS plasma_applet_ktorrent DESTINATION ${PLUGIN_INSTALL_DIR})
	install(FILES plasma-applet-ktorrent.desktop DESTINATION ${SERVICES_INSTALL_DIR})

endif(NOT QT_VERSION_OK)
