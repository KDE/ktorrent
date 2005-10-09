INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kttorinfo
kttorinfo_SOURCES = main.cpp
kttorinfo_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kttorinfo_LDADD = $(LIB_KFILE) $(LIB_KIO) ../../libktorrent/libktorrent.la

