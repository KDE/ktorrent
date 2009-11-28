
set (magnet_HDR
	magnetdownloader.h
	magnetlink.h
	metadatadownload.h
)

install(FILES ${magnet_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/magnet COMPONENT Devel)
add_subdirectory(tests)