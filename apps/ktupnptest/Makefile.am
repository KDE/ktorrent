INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes) -I$(srcdir)/../..
METASOURCES = AUTO
bin_PROGRAMS = ktupnptest

ktupnptest_SOURCES = main.cpp
ktupnptest_LDADD = ../../plugins/upnp/libktupnp.la \
	../../libktorrent/libktorrent.la
