
add_subdirectory(tests)

set(net_HDR  
	socks.h
	bufferedsocket.h
	downloadthread.h
	address.h
	socketgroup.h
	portlist.h
	networkthread.h
	socket.h
	speed.h
	uploadthread.h
	socketmonitor.h
	reverseresolver.h
	socketdevice.h
	poll.h
)

install(FILES ${net_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/net COMPONENT Devel)

