INCLUDES = -I$(srcdir)/.. $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kttorinfo ktcachecheck
kttorinfo_SOURCES = main.cpp
kttorinfo_LDADD = ../libtorrent/libtorrent.la \
	../libutil/libutil.la $(LIB_KIO) $(LIB_KFILE)

ktcachecheck_SOURCES = cachecheck.cpp cachechecker.cpp singlecachechecker.cpp \
	multicachechecker.cpp
ktcachecheck_LDADD = ../libtorrent/libtorrent.la \
	../libutil/libutil.la $(LIB_KFILE) $(LIB_KIO)
noinst_HEADERS = cachechecker.h singlecachechecker.h multicachechecker.h
