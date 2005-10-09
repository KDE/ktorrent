INCLUDES = -I$(srcdir)/. $(all_includes)
SUBDIRS = util torrent kademlia
METASOURCES = AUTO

lib_LTLIBRARIES = libktorrent.la
libktorrent_la_LDFLAGS = ${KDE_RPATH} $(all_libraries)
libktorrent_la_LDADD = torrent/libtorrent.la util/libutil.la kademlia/libkademlia.la

libktorrent_la_LIBADD = $(top_builddir)/libktorrent/util/libutil.la\
	$(top_builddir)/libktorrent/torrent/libtorrent.la\
	$(top_builddir)/libktorrent/kademlia/libkademlia.la
