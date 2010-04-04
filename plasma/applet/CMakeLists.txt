set(TASKMANAGER_FOUND FALSE)
FIND_PATH(TASKMANAGER_INCLUDE_DIR NAMES taskmanager.h  PATHS ${KDE4_INCLUDE_DIR}/taskmanager ${INCLUDE_INSTALL_DIR}/taskmanager.h)
FIND_LIBRARY(TASKMANAGER_LIBRARY NAMES taskmanager PATHS ${KDE4_LIB_DIR} ${LIB_INSTALL_DIR})

if(TASKMANAGER_INCLUDE_DIR AND TASKMANAGER_LIBRARY)
	set(TASKMANAGER_FOUND TRUE)
	message(STATUS "Found libtaskmanager: ${TASKMANAGER_LIBRARY} ")
	include_directories(${TASKMANAGER_INCLUDE_DIR})
	set(ktapplet_SRCS applet.cpp chunkbar.cpp fadingitem.cpp fadingnavigationwidget.cpp)
	
	kde4_add_ui_files(ktapplet_SRCS appletconfig.ui)
	
	kde4_add_plugin(plasma_applet_ktorrent ${ktapplet_SRCS})
	target_link_libraries(plasma_applet_ktorrent  ${PLASMA_LIBS} ${KDE4_KIO_LIBS} ${TASKMANAGER_LIBRARY} ${LIBKTORRENT_LIBRARIES} ktcore)
	
	install(TARGETS plasma_applet_ktorrent DESTINATION ${PLUGIN_INSTALL_DIR})
	install(FILES plasma-applet-ktorrent.desktop DESTINATION ${SERVICES_INSTALL_DIR})
endif(TASKMANAGER_INCLUDE_DIR AND TASKMANAGER_LIBRARY)

macro_log_feature(TASKMANAGER_FOUND "libtaskmanager" "libtaskmanager library and header files" "http://www.kde.org/" FALSE "" "libtaskmanager is needed for KTorrent Plasmoid")