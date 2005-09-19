INCLUDES = -I$(srcdir)/.. $(all_includes)
METASOURCES = AUTO

noinst_PROGRAMS = kttorinfo cachecheck
kttorinfo_SOURCES = main.cpp
kttorinfo_LDADD = $(top_builddir)/libtorrent/libtorrent.la \
	$(top_builddir)/libutil/libutil.la $(LIB_KIO) $(LIB_KFILE)

cachecheck_SOURCES = cachecheck.cpp cachechecker.cpp singlecachechecker.cpp \
	multicachechecker.cpp
cachecheck_LDADD = $(top_builddir)/libtorrent/libtorrent.la \
	$(top_builddir)/libutil/libutil.la $(LIB_KFILE) $(LIB_KIO)
noinst_HEADERS = cachechecker.h singlecachechecker.h multicachechecker.h
