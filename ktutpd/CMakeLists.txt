include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(ktutpd_SRC 
	main.cpp 
	utpdaemon.cpp
)

QT4_ADD_DBUS_ADAPTOR(ktutpd_SRC ${KTORRENT_DBUS_XML_DIR}/org.ktorrent.UTPDaemon.xml utpdaemon.h utp::UTPDaemon)

kde4_add_executable(ktutpd ${ktutpd_SRC})
target_link_libraries(
	ktutpd 
	${QT_QTCORE_LIBRARY} 
	${QT_QTDBUS_LIBRARY} 
	${QT_QTNETWORK_LIBRARY}
)

install(TARGETS ktutpd ${INSTALL_TARGETS_DEFAULT_ARGS})