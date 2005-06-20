INCLUDES = $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libtorrent.la
libtorrent_la_LDFLAGS = $(all_libraries)
noinst_HEADERS = bencoder.h sha1hash.h value.h bdecoder.h bnode.h error.h \
						torrent.h sha1hashgen.h chunkmanager.h chunk.h peer.h globals.h bitset.h \
						peermanager.h tracker.h downloader.h authenticate.h uploader.h torrentcontrol.h \
						choker.h chunkdownload.h timer.h speedestimater.h filereconstructor.h \
						urlencoder.h ptrlist.h peerdownloader.h request.h piece.h torrentmonitor.h peerid.h \
						announcelist.h peeruploader.h uploadcap.h packetreader.h ptrmap.h log.h packetwriter.h \
				packet.h file.h httptracker.h udptracker.h cache.h
libtorrent_la_SOURCES = bencoder.cpp sha1hash.cpp value.cpp bdecoder.cpp \
			bnode.cpp error.cpp torrent.cpp sha1hashgen.cpp chunkmanager.cpp chunk.cpp \
			peer.cpp globals.cpp bitset.cpp peermanager.cpp tracker.cpp downloader.cpp \
			authenticate.cpp uploader.cpp torrentcontrol.cpp choker.cpp chunkdownload.cpp timer.cpp \
			speedestimater.cpp filereconstructor.cpp urlencoder.cpp ptrlist.cpp peerdownloader.cpp \
			request.cpp piece.cpp torrentmonitor.cpp peerid.cpp announcelist.cpp \
			peeruploader.cpp uploadcap.cpp packetreader.cpp ptrmap.cpp log.cpp packetwriter.cpp \
			packet.cpp file.cpp httptracker.cpp udptracker.cpp cache.cpp

