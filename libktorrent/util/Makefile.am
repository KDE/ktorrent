INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libutil.la
libutil_la_LDFLAGS = $(all_libraries)


libutil_la_SOURCES = array.cpp autorotatelogjob.cpp bitset.cpp error.cpp \
	file.cpp fileops.cpp functions.cpp httprequest.cpp log.cpp mmapfile.cpp \
	profiler.cpp ptrmap.cpp sha1hash.cpp sha1hashgen.cpp timer.cpp urlencoder.cpp \
	waitjob.cpp




KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
noinst_HEADERS = autorotatelogjob.h profiler.h
