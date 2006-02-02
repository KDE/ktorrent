INCLUDES = -I$(srcdir)/. $(all_includes)
SUBDIRS = util torrent kademlia interfaces migrate mse
METASOURCES = AUTO

lib_LTLIBRARIES = libktorrent.la
libktorrent_la_LDFLAGS = ${KDE_RPATH} $(all_libraries)



noinst_HEADERS = pluginmanager.h functions.h expandablewidget.h \
		pluginmanagerprefpage.h ktversion.h
libktorrent_la_SOURCES = pluginmanager.cpp functions.cpp expandablewidget.cpp \
	pluginmanagerprefpage.cpp pluginmanagerwidget.ui
libktorrent_la_LIBADD = ../libktorrent/mse/libmse.la \
	../libktorrent/migrate/libmigrate.la ../libktorrent/util/libutil.la ../libktorrent/torrent/libtorrent.la \
	../libktorrent/kademlia/libkademlia.la ../libktorrent/interfaces/libinterfaces.la $(LIB_KPARTS)
