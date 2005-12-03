INCLUDES = -I$(srcdir)/.. $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libtorrent.la
libtorrent_la_LDFLAGS = $(all_libraries)
noinst_HEADERS = bencoder.h value.h bdecoder.h bnode.h torrent.h chunkmanager.h \
				chunk.h peer.h globals.h peermanager.h tracker.h downloader.h 	authenticate.h \
			uploader.h torrentcontrol.h choker.h chunkdownload.h speedestimater.h \
				peerdownloader.h request.h piece.h peerid.h announcelist.h peeruploader.h uploadcap.h \
				packetreader.h packetwriter.h packet.h httptracker.h udptracker.h cache.h \
				singlefilecache.h multifilecache.h torrentcreator.h downloadcap.h torrentfile.h \
				chunkselector.h server.h serverauthenticate.h authenticatebase.h udptrackersocket.h \
				upspeedestimater.h ipblocklist.h queuemanager.h chunkcounter.h oldchokealgorithm.h \
	newchokealgorithm.h
libtorrent_la_SOURCES = bencoder.cpp value.cpp bdecoder.cpp bnode.cpp \
				torrent.cpp chunkmanager.cpp chunk.cpp peer.cpp globals.cpp 	peermanager.cpp \
			tracker.cpp downloader.cpp authenticate.cpp uploader.cpp 	torrentcontrol.cpp \
			choker.cpp chunkdownload.cpp speedestimater.cpp peerdownloader.cpp 	request.cpp \
			piece.cpp peerid.cpp announcelist.cpp peeruploader.cpp uploadcap.cpp \
				packetreader.cpp packetwriter.cpp packet.cpp httptracker.cpp udptracker.cpp cache.cpp \
				singlefilecache.cpp multifilecache.cpp torrentcreator.cpp downloadcap.cpp torrentfile.cpp \
				chunkselector.cpp server.cpp serverauthenticate.cpp authenticatebase.cpp \
				udptrackersocket.cpp upspeedestimater.cpp ipblocklist.cpp queuemanager.cpp chunkcounter.cpp \
	oldchokealgorithm.cpp newchokealgorithm.cpp


