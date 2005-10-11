INCLUDES = -I$(srcdir)/. $(all_includes)
SUBDIRS = util torrent kademlia interfaces
METASOURCES = AUTO

lib_LTLIBRARIES = libktorrent.la
libktorrent_la_LDFLAGS = ${KDE_RPATH} $(all_libraries)
# libktorrent_la_LDADD = torrent/libtorrent.la util/libutil.la kademlia/libkademlia.la

libktorrent_la_LIBADD = $(top_builddir)/ktorrent/libktorrent/interfaces/libinterfaces.la \
	$(top_builddir)/ktorrent/libktorrent/util/libutil.la $(top_builddir)/ktorrent/libktorrent/torrent/libtorrent.la \
	$(top_builddir)/ktorrent/libktorrent/kademlia/libkademlia.la $(LIB_KPARTS)



noinst_HEADERS = pluginmanager.h
libktorrent_la_SOURCES = pluginmanager.cpp
