INCLUDES = -I$(srcdir)/../libktorrent -I$(srcdir)/.. $(all_includes)
METASOURCES = AUTO
noinst_HEADERS = unittest.h testrunner.h upnpparsedescriptiontest.h \
			upnpparseresponsetest.h dhtmsgparsetest.h biginttest.h
bin_PROGRAMS = ktutester
ktutester_LDFLAGS = $(KDE_RPATH) $(all_libraries)
ktutester_SOURCES = unittest.cpp testrunner.cpp main.cpp \
			upnpparsedescriptiontest.cpp upnpparseresponsetest.cpp dhtmsgparsetest.cpp biginttest.cpp
ktutester_LDADD = $(top_builddir)/ktorrent/plugins/upnp/libktupnp.la \
	$(top_builddir)/ktorrent/libktorrent/libktorrent.la
