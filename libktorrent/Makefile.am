INCLUDES = -I$(srcdir)/. $(all_includes)
SUBDIRS = util torrent kademlia interfaces
METASOURCES = AUTO

lib_LTLIBRARIES = libktorrent.la
libktorrent_la_LDFLAGS = ${KDE_RPATH} $(all_libraries)



noinst_HEADERS = pluginmanager.h functions.h
libktorrent_la_SOURCES = pluginmanager.cpp functions.cpp
libktorrent_la_LIBADD = ../libktorrent/util/libutil.la \
	../libktorrent/torrent/libtorrent.la ../libktorrent/kademlia/libkademlia.la \
	../libktorrent/interfaces/libinterfaces.la $(LIB_KPARTS)
