INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kttorinfo
kttorinfo_SOURCES = main.cpp
kttorinfo_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kttorinfo_LDADD = ../../libktorrent/libktorrent.la $(LIB_KPARTS) $(LIB_KFILE) \
	$(LIB_KIO)

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
