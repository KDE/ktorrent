INCLUDES = -I$(srcdir)/.. -I$(srcdir)/. $(all_includes)
METASOURCES = AUTO
libnet_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libnet.la
noinst_HEADERS = socket.h address.h socketmonitor.h circularbuffer.h \
	bufferedsocket.h speed.h
libnet_la_SOURCES = socket.cpp address.cpp socketmonitor.cpp circularbuffer.cpp \
	bufferedsocket.cpp speed.cpp
