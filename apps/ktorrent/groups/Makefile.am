INCLUDES = -I$(srcdir)/../../../libktorrent $(all_includes)
METASOURCES = AUTO
libgroups_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libgroups.la
noinst_HEADERS = group.h allgroup.h downloadgroup.h uploadgroup.h groupview.h \
	torrentgroup.h groupmanager.h torrentdrag.h queueddownloadsgroup.h \
	queueduploadsgroup.h userdownloadsgroup.h useruploadsgroup.h
libgroups_la_SOURCES = group.cpp allgroup.cpp downloadgroup.cpp uploadgroup.cpp \
				groupview.cpp torrentgroup.cpp groupmanager.cpp torrentdrag.cpp \
	queueddownloadsgroup.cpp queueduploadsgroup.cpp userdownloadsgroup.cpp useruploadsgroup.cpp
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
