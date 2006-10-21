INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libutil.la
libutil_la_LDFLAGS = $(all_libraries)


libutil_la_SOURCES = array.cpp file.cpp functions.cpp ptrmap.cpp \
		sha1hashgen.cpp urlencoder.cpp error.cpp fileops.cpp log.cpp sha1hash.cpp timer.cpp \
		bitset.cpp mmapfile.cpp httprequest.cpp waitjob.cpp profiler.cpp




KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
noinst_HEADERS = profiler.h
