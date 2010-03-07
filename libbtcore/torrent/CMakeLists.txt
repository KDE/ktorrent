set (torrent_HDR
	torrent.h
	advancedchokealgorithm.h
	peersourcemanager.h
	choker.h
	server.h
	torrentcontrol.h
	uploader.h
	torrentcreator.h
	timeestimator.h
	torrentfile.h
	statsfile.h
	globals.h
	torrentstats.h
	job.h
	jobqueue.h
)

install(FILES ${torrent_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/torrent COMPONENT Devel)

add_subdirectory(tests)