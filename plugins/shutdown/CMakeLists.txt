set(KWORKSPACE_FOUND FALSE)
FIND_PATH(KWORKSPACE_INCLUDE_DIR NAMES kworkspace.h PATH_SUFFIXES kworkspace5)
FIND_LIBRARY(KWORKSPACE_LIBRARY NAMES kworkspace5)

if(KWORKSPACE_INCLUDE_DIR AND KWORKSPACE_LIBRARY)
	set(KWORKSPACE_FOUND TRUE)
	message(STATUS "Found libkworkspace: ${KWORKSPACE_LIBRARY} ")
	include_directories(${KWORKSPACE_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	set(ktshutdownplugin_SRC shutdownruleset.cpp shutdowntorrentmodel.cpp shutdowndlg.cpp shutdownplugin.cpp)

	set(screensaver_xml ${KTORRENT_DBUS_XML_DIR}/org.freedesktop.ScreenSaver.xml)
	qt5_add_dbus_interface(ktshutdownplugin_SRC ${screensaver_xml} screensaver_interface)
	ki18n_wrap_ui(ktshutdownplugin_SRC shutdowndlg.ui)
	#kconfig_add_kcfg_files(ktshutdownplugin_SRC logviewerpluginsettings.kcfgc)
	add_library(ktshutdownplugin MODULE ${ktshutdownplugin_SRC})
	
	target_link_libraries(
		ktshutdownplugin
		ktcore
		${LIBKTORRENT_LIBRARIES}
		${KWORKSPACE_LIBRARY}
		KF5::Parts)
	if (NEW_SOLID_POWERMANAGEMENT)
		target_link_libraries(
			ktshutdownplugin
			)
	else (NEW_SOLID_POWERMANAGEMENT)
		target_link_libraries(
			ktshutdownplugin
			solidcontrol)
	endif (NEW_SOLID_POWERMANAGEMENT)
	install(TARGETS ktshutdownplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
	install(FILES ktshutdownplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
	install(FILES ktshutdownpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
endif(KWORKSPACE_INCLUDE_DIR AND KWORKSPACE_LIBRARY)
	
macro_log_feature(KWORKSPACE_FOUND "libkworkspace" "libkworkspace library and header files" "http://www.kde.org/" FALSE "" "libkworkspace is needed for shutdown plugin")



