include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${QCA2_INCLUDE_DIR})

set(libbtcore_SRC 
	util/bitset.cpp
	util/timer.cpp
	util/urlencoder.cpp
	util/sha1hashgen.cpp
	util/sha1hash.cpp
	util/functions.cpp
	util/ptrmap.cpp
	util/array.cpp
	util/error.cpp
	util/file.cpp
	util/fileops.cpp
	util/log.cpp
	util/waitjob.cpp
	util/autorotatelogjob.cpp
	
	bcodec/bdecoder.cpp  
	bcodec/bencoder.cpp  
	bcodec/bnode.cpp  
	bcodec/value.cpp

	net/address.cpp  
	net/bufferedsocket.cpp  
	net/portlist.cpp  
	net/socket.cpp  
	net/socketmonitor.cpp  
	net/speed.cpp
	net/uploadthread.cpp
	net/downloadthread.cpp
	net/networkthread.cpp
	net/socketgroup.cpp
	net/socks.cpp
	
	mse/bigint.cpp  
	mse/functions.cpp  
	mse/rc4encryptor.cpp  
	mse/streamsocket.cpp
	mse/encryptedauthenticate.cpp
	mse/encryptedserverauthenticate.cpp
	
	peer/authenticatebase.cpp
	peer/authenticate.cpp
	peer/serverauthenticate.cpp
	peer/authenticationmonitor.cpp
	peer/chunkcounter.cpp
	peer/peerid.cpp
	peer/peer.cpp
	peer/peermanager.cpp
	peer/peerdownloader.cpp
	peer/peeruploader.cpp
	peer/packetwriter.cpp
	peer/packetreader.cpp
	peer/utpex.cpp

	download/piece.cpp
	download/request.cpp
	download/packet.cpp
	download/httpdownloader.cpp
	download/chunkdownload.cpp
	download/chunkselector.cpp
	download/downloader.cpp

	interfaces/piecedownloader.cpp
	interfaces/peerinterface.cpp
	interfaces/peersource.cpp
	interfaces/torrentinterface.cpp
	interfaces/torrentfileinterface.cpp
	interfaces/trackerslist.cpp
	interfaces/logmonitorinterface.cpp
	interfaces/ipblockinginterface.cpp
	interfaces/exitoperation.cpp
	interfaces/chunkdownloadinterface.cpp
	interfaces/monitorinterface.cpp
	interfaces/queuemanagerinterface.cpp
	interfaces/chunkselectorinterface.cpp

	torrent/globals.cpp
	torrent/server.cpp
	torrent/torrent.cpp
	torrent/torrentfile.cpp
	torrent/ipblocklist.cpp
	torrent/peersourcemanager.cpp
	torrent/uploader.cpp
	torrent/timeestimator.cpp
	torrent/statsfile.cpp
	torrent/choker.cpp
	torrent/advancedchokealgorithm.cpp
	torrent/torrentcontrol.cpp
	torrent/torrentcreator.cpp

	dht/announcetask.cpp  
	dht/dht.cpp                
	dht/kclosestnodessearch.cpp  
	dht/nodelookup.cpp  
	dht/rpcmsg.cpp     
	dht/taskmanager.cpp
	dht/database.cpp      
	dht/dhttrackerbackend.cpp  
	dht/key.cpp                  
	dht/pack.cpp        
	dht/rpcserver.cpp
	dht/dhtbase.cpp       
	dht/kbucket.cpp            
	dht/node.cpp                 
	dht/rpccall.cpp     
	dht/task.cpp

	diskio/cache.cpp      
	diskio/chunk.cpp         
	diskio/dndfile.cpp         
	diskio/singlefilecache.cpp
	diskio/cachefile.cpp  
	diskio/chunkmanager.cpp  
	diskio/multifilecache.cpp
	diskio/preallocationthread.cpp
	diskio/movedatafilesjob.cpp

	tracker/httptracker.cpp  
	tracker/tracker.cpp  
	tracker/udptracker.cpp  
	tracker/udptrackersocket.cpp
	
	datachecker/datachecker.cpp          
	datachecker/datacheckerthread.cpp  
	datachecker/singledatachecker.cpp
	datachecker/datacheckerlistener.cpp  
	datachecker/multidatachecker.cpp
	
	migrate/cachemigrate.cpp  
	migrate/ccmigrate.cpp  
	migrate/migrate.cpp
	
	btversion.cpp
)


# kde4_add_ui_files(libbtcore_SRC ${libbtcore_UI})
# kde4_add_kcfg_files(libbtcore_SRC settings.kcfgc)
kde4_add_library(btcore SHARED ${libbtcore_SRC})

target_link_libraries(btcore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${GMP_LIBRARIES} ${QCA2_LIBRARIES})
set_target_properties(btcore PROPERTIES VERSION 3.0.0 SOVERSION 3 )
install(TARGETS btcore  DESTINATION ${LIB_INSTALL_DIR} )

set(net_HDR  
	./net/socks.h
	./net/bufferedsocket.h
	./net/downloadthread.h
	./net/address.h
	./net/socketgroup.h
	./net/portlist.h
	./net/networkthread.h
	./net/socket.h
	./net/speed.h
	./net/uploadthread.h
	./net/socketmonitor.h
)

set (mse_HDR 
	./mse/encryptedserverauthenticate.h
	./mse/functions.h
	./mse/rc4encryptor.h
	./mse/encryptedauthenticate.h
	./mse/bigint.h
	./mse/streamsocket.h
)

set (interfaces_HDR
	./interfaces/peerinterface.h
	./interfaces/queuemanagerinterface.h
	./interfaces/exitoperation.h
	./interfaces/chunkdownloadinterface.h
	./interfaces/trackerslist.h
	./interfaces/torrentinterface.h
	./interfaces/logmonitorinterface.h
	./interfaces/piecedownloader.h
	./interfaces/monitorinterface.h
	./interfaces/ipblockinginterface.h
	./interfaces/torrentfileinterface.h
	./interfaces/peersource.h
	./interfaces/chunkselectorinterface.h
)

set (peer_HDR
	./peer/peeruploader.h
	./peer/packetwriter.h
	./peer/peer.h
	./peer/peermanager.h
	./peer/authenticatebase.h
	./peer/authenticate.h
	./peer/utpex.h
	./peer/packetreader.h
	./peer/authenticationmonitor.h
	./peer/peerid.h
	./peer/peerdownloader.h
	./peer/chunkcounter.h
	./peer/serverauthenticate.h
)
	
set (util_HDR
	./util/autorotatelogjob.h
	./util/fileops.h
	./util/timer.h
	./util/ptrmap.h
	./util/log.h
	./util/waitjob.h
	./util/urlencoder.h
	./util/array.h
	./util/functions.h
	./util/file.h
	./util/constants.h
	./util/bitset.h
	./util/sha1hash.h
	./util/sha1hashgen.h
	./util/error.h
)
	
set (download_HDR
	./download/chunkdownload.h
	./download/downloader.h
	./download/httpdownloader.h
	./download/request.h
	./download/piece.h
	./download/packet.h
	./download/chunkselector.h
)

set (btcore_HDR 
	btcore_export.h
	btversion.h
)
	
set (bcodec_HDR
	./bcodec/bencoder.h
	./bcodec/bnode.h
	./bcodec/bdecoder.h
	./bcodec/value.h
)
	
set (diskio_HDR  
	./diskio/cache.h
	./diskio/cachefile.h
	./diskio/singlefilecache.h
	./diskio/preallocationthread.h
	./diskio/movedatafilesjob.h
	./diskio/chunkmanager.h
	./diskio/dndfile.h
	./diskio/chunk.h
	./diskio/multifilecache.h
)
	
set (torrent_HDR
	./torrent/torrent.h
	./torrent/advancedchokealgorithm.h
	./torrent/peersourcemanager.h
	./torrent/ipblocklist.h
	./torrent/choker.h
	./torrent/server.h
	./torrent/torrentcontrol.h
	./torrent/uploader.h
	./torrent/torrentcreator.h
	./torrent/timeestimator.h
	./torrent/torrentfile.h
	./torrent/statsfile.h
	./torrent/globals.h
)
	
set (tracker_HDR
	./tracker/tracker.h
	./tracker/udptracker.h
	./tracker/udptrackersocket.h
	./tracker/httptracker.h
)

set (datachecker_HDR
	./datachecker/datacheckerthread.h
	./datachecker/datachecker.h
	./datachecker/singledatachecker.h
	./datachecker/datacheckerlistener.h
	./datachecker/multidatachecker.h
)

set (dht_HDR
	./dht/dhtbase.h
	./dht/rpcserver.h
	./dht/key.h
	./dht/pack.h
	./dht/rpcmsg.h
	./dht/dhttrackerbackend.h
	./dht/announcetask.h
	./dht/rpccall.h
	./dht/kclosestnodessearch.h
	./dht/dht.h
	./dht/node.h
	./dht/task.h
	./dht/nodelookup.h
	./dht/taskmanager.h
	./dht/kbucket.h
	./dht/database.h
)

set (migrate_HDR
	./migrate/ccmigrate.h
	./migrate/migrate.h
	./migrate/cachemigrate.h
)

install(FILES ${tracker_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/tracker )
install(FILES ${torrent_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/torrent )
install(FILES ${diskio_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/diskio )
install(FILES ${bcodec_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/bcodec )
install(FILES ${net_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/net )
install(FILES ${mse_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/mse )
install(FILES ${interfaces_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/interfaces )
install(FILES ${util_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/util )
install(FILES ${peer_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/peer )
install(FILES ${datachecker_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/datachecker )
install(FILES ${dht_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/dht )
install(FILES ${migrate_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore/migrate )
install(FILES ${btcore_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore )
