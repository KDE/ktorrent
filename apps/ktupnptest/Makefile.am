INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes) -I$(srcdir)/../..
METASOURCES = AUTO
bin_PROGRAMS = ktupnptest

ktupnptest_SOURCES = main.cpp upnptestapp.cpp mainwidget.ui
ktupnptest_LDADD = ../../plugins/upnp/libktupnp.la \
	../../libktorrent/libktorrent.la
noinst_HEADERS = upnptestapp.h
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
