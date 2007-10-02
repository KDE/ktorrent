include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(libktorrent_SRC 
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

	mse/bigint.cpp  
	mse/functions.cpp  
	mse/rc4encryptor.cpp  
	mse/streamsocket.cpp
	mse/encryptedauthenticate.cpp
	mse/encryptedserverauthenticate.cpp

	datachecker/datachecker.cpp          
	datachecker/datacheckerthread.cpp  
	datachecker/singledatachecker.cpp
	datachecker/datacheckerlistener.cpp  
	datachecker/multidatachecker.cpp

	interfaces/logmonitorinterface.cpp
	interfaces/torrentfileinterface.cpp
	interfaces/trackerslist.cpp
	interfaces/torrentinterface.cpp
	interfaces/peersource.cpp
	interfaces/peerinterface.cpp
	interfaces/ipblockinginterface.cpp
	interfaces/exitoperation.cpp
	interfaces/chunkdownloadinterface.cpp
	interfaces/monitorinterface.cpp
	interfaces/functions.cpp
	interfaces/plugin.cpp
	interfaces/guiinterface.cpp
	interfaces/coreinterface.cpp
	interfaces/prefpageinterface.cpp
	interfaces/filetreeitem.cpp
	interfaces/filetreediritem.cpp
	interfaces/piecedownloader.cpp

	migrate/cachemigrate.cpp  
	migrate/ccmigrate.cpp  
	migrate/migrate.cpp

	util/timer.cpp
	util/urlencoder.cpp
	util/sha1hashgen.cpp
	util/sha1hash.cpp
	util/functions.cpp
	util/ptrmap.cpp
	util/array.cpp
	util/bitset.cpp
	util/error.cpp
	util/file.cpp
	util/fileops.cpp
	util/log.cpp
	util/waitjob.cpp
	util/autorotatelogjob.cpp
	util/mmapfile.cpp

	tracker/httptracker.cpp  
	tracker/tracker.cpp  
	tracker/udptracker.cpp  
	tracker/udptrackersocket.cpp

	diskio/cache.cpp      
	diskio/chunk.cpp         
	diskio/dndfile.cpp         
	diskio/singlefilecache.cpp
	diskio/cachefile.cpp  
	diskio/chunkmanager.cpp  
	diskio/multifilecache.cpp
	diskio/preallocationthread.cpp

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

	torrent/torrentfile.cpp
	torrent/torrent.cpp
	torrent/ipblocklist.cpp
	torrent/globals.cpp
	torrent/server.cpp
	torrent/peersourcemanager.cpp
	torrent/uploader.cpp
	torrent/timeestimator.cpp
	torrent/queuemanager.cpp
	torrent/statsfile.cpp
	torrent/choker.cpp
	torrent/advancedchokealgorithm.cpp
	torrent/torrentcontrol.cpp
	torrent/torrentcreator.cpp

	
	peer/piece.cpp
	peer/request.cpp
	peer/packet.cpp
	peer/chunkcounter.cpp
	peer/peerid.cpp
	peer/authenticatebase.cpp
	peer/authenticate.cpp
	peer/serverauthenticate.cpp
	peer/authenticationmonitor.cpp
	peer/peer.cpp
	peer/peermanager.cpp
	peer/peerdownloader.cpp
	peer/peeruploader.cpp
	peer/packetwriter.cpp
	peer/packetreader.cpp
	peer/utpex.cpp

	groups/group.cpp
	groups/torrentgroup.cpp
	groups/allgroup.cpp
	groups/downloadgroup.cpp
	groups/uploadgroup.cpp
	groups/activedownloadsgroup.cpp  
	groups/activegroup.cpp  
	groups/activeuploadsgroup.cpp
	groups/inactivedownloadsgroup.cpp  
	groups/inactivegroup.cpp  
	groups/inactiveuploadsgroup.cpp
	groups/queueddownloadsgroup.cpp  
	groups/queueduploadsgroup.cpp  
	groups/userdownloadsgroup.cpp  
	groups/useruploadsgroup.cpp
	groups/groupmanager.cpp
	
	download/httpdownloader.cpp
	download/chunkdownload.cpp
	download/chunkselector.cpp
	download/downloader.cpp

	ktorrent.kcfg
	labelviewitem.ui
	labelview.cpp
	pluginmanager.cpp
	pluginmanagerprefpage.cpp
)

set(libktorrent_UI pluginmanagerwidget.ui labelviewitem.ui)

kde4_add_ui_files(libktorrent_SRC ${libktorrent_UI})
kde4_add_kcfg_files(libktorrent_SRC settings.kcfgc)
kde4_add_library(ktcore SHARED ${libktorrent_SRC})

# target_link_libraries(ktorrent ${KDE4_KPARTS_LIBS} -lgmp)
target_link_libraries(ktcore ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KPARTS_LIBS} -lsolid -lgmp)
set_target_properties(ktcore PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS ktcore  DESTINATION ${LIB_INSTALL_DIR} )

