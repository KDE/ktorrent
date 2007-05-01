INCLUDES = -I$(srcdir)/.. -I$(srcdir)/. $(all_includes)
METASOURCES = AUTO
libnet_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libnet.la
noinst_HEADERS = address.h bufferedsocket.h circularbuffer.h downloadthread.h \
	networkthread.h portlist.h socket.h socketmonitor.h speed.h uploadthread.h
libnet_la_SOURCES = address.cpp bufferedsocket.cpp circularbuffer.cpp \
	downloadthread.cpp networkthread.cpp portlist.cpp socket.cpp socketgroup.cpp \
	socketmonitor.cpp speed.cpp uploadthread.cpp
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
