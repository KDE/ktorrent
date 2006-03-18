INCLUDES = -I$(srcdir)/../libktorrent -I$(srcdir)/.. $(all_includes)
METASOURCES = AUTO
noinst_HEADERS = unittest.h testrunner.h upnpparsedescriptiontest.h \
		upnpparseresponsetest.h dhtmsgparsetest.h
bin_PROGRAMS = ktutester
ktutester_LDFLAGS = $(KDE_RPATH) $(all_libraries)
ktutester_SOURCES = unittest.cpp testrunner.cpp main.cpp \
		upnpparsedescriptiontest.cpp upnpparseresponsetest.cpp dhtmsgparsetest.cpp
ktutester_LDADD = $(top_builddir)/plugins/upnp/libktupnp.la \
	$(top_builddir)/libktorrent/libktorrent.la
