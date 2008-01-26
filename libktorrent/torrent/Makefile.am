INCLUDES = -I$(top_builddir)/ktorrent/libktorrent -I$(top_builddir)/libktorrent \
	-I$(srcdir)/.. $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libtorrent.la
libtorrent_la_LDFLAGS = $(all_libraries)
noinst_HEADERS = advancedchokealgorithm.h announcelist.h authenticate.h \
	authenticatebase.h authenticationmonitor.h bdecoder.h bencoder.h bnode.h cache.h \
	cachefile.h cap.h choker.h chunk.h chunkcounter.h chunkdownload.h chunkmanager.h \
	chunkselector.h dndfile.h downloadcap.h downloader.h globals.h httptracker.h \
	ipblocklist.h movedatafilesjob.h multifilecache.h newchokealgorithm.h \
	oldchokealgorithm.h packet.h packetreader.h packetwriter.h peer.h peerdownloader.h peerid.h \
	peermanager.h peersourcemanager.h peeruploader.h piece.h preallocationthread.h \
	queuemanager.h request.h server.h serverauthenticate.h singlefilecache.h \
	speedestimater.h statsfile.h timeestimator.h torrent.h torrentcontrol.h torrentcreator.h \
	torrentfile.h tracker.h udptracker.h udptrackersocket.h uploadcap.h uploader.h \
	upspeedestimater.h utpex.h value.h

libtorrent_la_SOURCES = advancedchokealgorithm.cpp announcelist.cpp \
	authenticate.cpp authenticatebase.cpp authenticationmonitor.cpp bdecoder.cpp \
	bencoder.cpp bnode.cpp cache.cpp cachefile.cpp cap.cpp choker.cpp chunk.cpp \
	chunkcounter.cpp chunkdownload.cpp chunkmanager.cpp chunkselector.cpp dndfile.cpp \
	downloadcap.cpp downloader.cpp globals.cpp httptracker.cpp ipblocklist.cpp \
	movedatafilesjob.cpp multifilecache.cpp newchokealgorithm.cpp packet.cpp packetreader.cpp \
	packetwriter.cpp peer.cpp peerdownloader.cpp peerid.cpp peermanager.cpp \
	peersourcemanager.cpp peeruploader.cpp piece.cpp preallocationthread.cpp queuemanager.cpp \
	request.cpp server.cpp serverauthenticate.cpp singlefilecache.cpp \
	speedestimater.cpp statsfile.cpp timeestimator.cpp torrent.cpp torrentcontrol.cpp \
	torrentcreator.cpp torrentfile.cpp tracker.cpp udptracker.cpp udptrackersocket.cpp \
	uploadcap.cpp uploader.cpp upspeedestimater.cpp utpex.cpp value.cpp

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
