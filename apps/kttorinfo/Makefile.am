INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kttorinfo
kttorinfo_SOURCES = main.cpp
kttorinfo_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kttorinfo_LDADD = $(top_builddir)/libktorrent/util/libutil.la \
	$(top_builddir)/libktorrent/torrent/libtorrent.la $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KIO)

