INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libutil.la
libutil_la_LDFLAGS = $(all_libraries)

noinst_HEADERS = array.h error.h fileops.h log.h sha1hash.h timer.h constants.h \
	file.h functions.h ptrmap.h sha1hashgen.h urlencoder.h profile.h bitset.h \
	mmapfile.h

libutil_la_SOURCES = array.cpp file.cpp functions.cpp ptrmap.cpp \
	sha1hashgen.cpp urlencoder.cpp error.cpp fileops.cpp log.cpp sha1hash.cpp timer.cpp \
	profile.cpp bitset.cpp mmapfile.cpp



