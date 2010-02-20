add_subdirectory(tests)

set (util_HDR
	autorotatelogjob.h
	fileops.h
	timer.h
	ptrmap.h
	log.h
	waitjob.h
	urlencoder.h
	array.h
	functions.h
	file.h
	constants.h
	bitset.h
	sha1hash.h
	sha1hashgen.h
	error.h
	win32.h
	logsystemmanager.h
	compressfilejob.h
	decompressfilejob.h
	extractfilejob.h
	pipe.h
	circularbuffer.h
)

install(FILES ${util_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/util COMPONENT Devel)