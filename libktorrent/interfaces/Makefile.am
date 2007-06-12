INCLUDES = -I$(srcdir)/.. $(all_includes)
METASOURCES = AUTO
libinterfaces_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libinterfaces.la
noinst_HEADERS = plugin.h coreinterface.h guiinterface.h torrentinterface.h \
		monitorinterface.h chunkdownloadinterface.h peerinterface.h torrentfileinterface.h \
		filetreeitem.h filetreediritem.h logmonitorinterface.h ipblockinginterface.h \
		trackerslist.h peersource.h exitoperation.h 
libinterfaces_la_SOURCES = plugin.cpp coreinterface.cpp guiinterface.cpp \
			prefpageinterface.cpp torrentinterface.cpp monitorinterface.cpp chunkdownloadinterface.cpp \
			peerinterface.cpp torrentfileinterface.cpp filetreeitem.cpp filetreediritem.cpp \
			functions.cpp logmonitorinterface.cpp ipblockinginterface.cpp trackerslist.cpp \
			peersource.cpp exitoperation.cpp 

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
