INCLUDES = -I$(top_srcdir)/ktorrent $(all_includes)
METASOURCES = AUTO

noinst_PROGRAMS = kttorinfo
kttorinfo_SOURCES = main.cpp
kttorinfo_LDADD = $(top_builddir)/ktorrent/libtorrent/libtorrent.la \
	$(top_builddir)/ktorrent/libutil/libutil.la $(LIB_KIO) $(LIB_KFILE)
