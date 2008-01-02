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
set_target_properties(btcore PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS btcore  DESTINATION ${LIB_INSTALL_DIR} )

set(net_HDR  
	./net/address.h
	./net/portlist.h
	./net/socketmonitor.h
	./net/socket.h
)

set (mse_HDR ./mse/streamsocket.h)

set (interfaces_HDR
	./interfaces/queuemanagerinterface.h
	./interfaces/ipblockinginterface.h
	./interfaces/exitoperation.h
	./interfaces/trackerslist.h
	./interfaces/logmonitorinterface.h
	./interfaces/torrentfileinterface.h
	./interfaces/peersource.h
	./interfaces/peersource.h
        ./interfaces/peerinterface.h
	./interfaces/monitorinterface.h
	./interfaces/torrentinterface.h
	./interfaces/torrentinterface.h
    ./interfaces/chunkdownloadinterface.h
)

set (peer_HDR
	./peer/authenticationmonitor.h
	./peer/peermanager.h
	./peer/peerid.h
)
	
set (util_HDR
	./util/sha1hash.h
	./util/fileops.h
	./util/bitset.h
	./util/ptrmap.h
	./util/log.h
	./util/functions.h
	./util/error.h
	./util/timer.h
	./util/file.h
	./util/waitjob.h
	./util/array.h
	./util/sha1hashgen.h
	./util/constants.h
)
	
set (download_HDR
	./download/downloader.h
)

set (btcore_HDR 
	btcore_export.h
	btversion.h
)
	
set (bcodec_HDR
	./bcodec/bencoder.h
	./bcodec/bdecoder.h
	./bcodec/value.h
	./bcodec/bnode.h
)
	
set (diskio_HDR  
	./diskio/chunkmanager.h
	./diskio/cache.h
)
	
set (torrent_HDR
	./torrent/torrentfile.h
	./torrent/torrentcontrol.h
	./torrent/choker.h
	./torrent/ipblocklist.h
	./torrent/torrent.h
	./torrent/server.h
	./torrent/globals.h
	./torrent/torrentcreator.h
)
	
set (tracker_HDR
	./tracker/httptracker.h
	./tracker/udptrackersocket.h
	./tracker/tracker.h
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
install(FILES ${btcore_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libbtcore )
