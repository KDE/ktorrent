INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = ktcachecheck

ktcachecheck_SOURCES = cachecheck.cpp cachechecker.cpp singlecachechecker.cpp \
	multicachechecker.cpp
ktcachecheck_LDFLAGS = $(KDE_RPATH) $(all_libraries)
ktcachecheck_LDADD = $(LIB_KPARTS) ../../libktorrent/libktorrent.la \
	$(LIB_KFILE) $(LIB_KIO)
noinst_HEADERS = cachechecker.h singlecachechecker.h multicachechecker.h
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
