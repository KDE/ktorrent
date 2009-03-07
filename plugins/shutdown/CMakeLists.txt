set(KWORKSPACE_FOUND FALSE)
FIND_PATH(KWORKSPACE_INCLUDE_DIR NAMES kworkspace.h  PATHS ${KDE4_INCLUDE_DIR}/kworkspace ${INCLUDE_INSTALL_DIR}/kworkspace.h)
FIND_LIBRARY(KWORKSPACE_LIBRARY NAMES kworkspace PATHS ${KDE4_LIB_DIR} ${LIB_INSTALL_DIR})

if(KWORKSPACE_INCLUDE_DIR AND KWORKSPACE_LIBRARY)
	set(KWORKSPACE_FOUND TRUE)
	message(STATUS "Found libkworkspace: ${KWORKSPACE_LIBRARY} ")
	include_directories(${KWORKSPACE_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	set(ktshutdownplugin_SRC shutdownplugin.cpp)

	set(screensaver_xml org.freedesktop.ScreenSaver.xml)
	QT4_ADD_DBUS_INTERFACE(ktshutdownplugin_SRC ${screensaver_xml} screensaver_interface)
	#kde4_add_ui_files(ktshutdownplugin_SRC logprefwidget.ui)
	#kde4_add_kcfg_files(ktshutdownplugin_SRC logviewerpluginsettings.kcfgc)
	kde4_add_plugin(ktshutdownplugin ${ktshutdownplugin_SRC})
	
	target_link_libraries(
		ktshutdownplugin
		ktcore
		btcore
		solidcontrol
		kworkspace
		${KDE4_KPARTS_LIBS}
		${QT_QT3SUPPORT_LIBRARY}
		)
	install(TARGETS ktshutdownplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
	install(FILES ktshutdownplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
endif(KWORKSPACE_INCLUDE_DIR AND KWORKSPACE_LIBRARY)
	
macro_log_feature(KWORKSPACE_FOUND "libkworkspace" "libkworkspace library and header files" "http://www.kde.org/" FALSE "" "libkworkspace is needed for shutdown plugin")



