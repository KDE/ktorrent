INCLUDES = -I$(srcdir)/.. -I$(srcdir)/. $(all_includes)
METASOURCES = AUTO
libdatachecker_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libdatachecker.la
libdatachecker_la_SOURCES = datachecker.cpp multidatachecker.cpp \
		singledatachecker.cpp datacheckerlistener.cpp datacheckerthread.cpp
noinst_HEADERS = datacheckerlistener.h datacheckerthread.h
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
