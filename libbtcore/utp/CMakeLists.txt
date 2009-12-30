set (utp_HDR
	utpserver.h
	utpprotocol.h
)

install(FILES ${utp_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/utp COMPONENT Devel)
#add_subdirectory(tests)