set (utp_HDR
	utpserver.h
	utpserverthread.h
	utpprotocol.h
	utpsocket.h
	connection.h
	localwindow.h
	timevalue.h
	pollpipe.h
	delaywindow.h
)

install(FILES ${utp_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/utp COMPONENT Devel)
#add_subdirectory(tests)
add_subdirectory(tests)