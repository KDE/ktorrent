INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes) -I$(srcdir)/../..
METASOURCES = AUTO
bin_PROGRAMS = ktupnptest

ktupnptest_SOURCES = main.cpp
ktupnptest_LDADD = $(top_builddir)/plugins/upnp/libktupnp.la \
	$(top_builddir)/libktorrent/libktorrent.la
