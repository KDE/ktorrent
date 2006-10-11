INCLUDES = -I$(srcdir)/. $(all_includes)
SUBDIRS = util torrent kademlia interfaces migrate mse datachecker net
METASOURCES = AUTO

lib_LTLIBRARIES = libktorrent.la
libktorrent_la_LDFLAGS = ${KDE_RPATH} $(all_libraries)



noinst_HEADERS = pluginmanager.h functions.h expandablewidget.h \
		pluginmanagerprefpage.h ktversion.h
libktorrent_la_SOURCES = pluginmanager.cpp functions.cpp expandablewidget.cpp \
	pluginmanagerprefpage.cpp pluginmanagerwidget.ui
libktorrent_la_LIBADD = ../libktorrent/net/libnet.la \
	../libktorrent/datachecker/libdatachecker.la ../libktorrent/mse/libmse.la ../libktorrent/migrate/libmigrate.la \
	../libktorrent/util/libutil.la ../libktorrent/torrent/libtorrent.la \
	../libktorrent/kademlia/libkademlia.la ../libktorrent/interfaces/libinterfaces.la $(LIB_KPARTS)
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
