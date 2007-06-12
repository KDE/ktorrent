INCLUDES = -I$(srcdir)/../../../libktorrent $(all_includes)
METASOURCES = AUTO
libgroups_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libgroups.la
noinst_HEADERS = activedownloadsgroup.h activegroup.h activeuploadsgroup.h \
	allgroup.h downloadgroup.h group.h groupmanager.h groupview.h \
	inactivedownloadsgroup.h inactivegroup.h inactiveuploadsgroup.h queueddownloadsgroup.h \
	queueduploadsgroup.h torrentdrag.h torrentgroup.h uploadgroup.h userdownloadsgroup.h \
	useruploadsgroup.h
libgroups_la_SOURCES = activedownloadsgroup.cpp activegroup.cpp \
	activeuploadsgroup.cpp allgroup.cpp downloadgroup.cpp group.cpp groupmanager.cpp groupview.cpp \
	inactivedownloadsgroup.cpp inactivegroup.cpp inactiveuploadsgroup.cpp queueddownloadsgroup.cpp \
	queueduploadsgroup.cpp torrentdrag.cpp torrentgroup.cpp uploadgroup.cpp userdownloadsgroup.cpp \
	useruploadsgroup.cpp
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
