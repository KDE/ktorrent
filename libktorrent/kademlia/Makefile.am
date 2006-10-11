INCLUDES = -I$(srcdir)/.. $(all_includes)
METASOURCES = AUTO
libkademlia_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libkademlia.la
noinst_HEADERS = key.h node.h kbucket.h rpccall.h rpcserver.h database.h dht.h \
								rpcmsg.h kclosestnodessearch.h nodelookup.h task.h pack.h \
					taskmanager.h announcetask.h dhttrackerbackend.h dhtbase.h
libkademlia_la_SOURCES = key.cpp node.cpp kbucket.cpp rpccall.cpp rpcserver.cpp \
								database.cpp dht.cpp rpcmsg.cpp kclosestnodessearch.cpp nodelookup.cpp task.cpp \
					pack.cpp taskmanager.cpp announcetask.cpp \
		dhttrackerbackend.cpp dhtbase.cpp
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
