INCLUDES = -I$(srcdir)/. $(all_includes)
SUBDIRS = util torrent kademlia interfaces migrate mse datachecker net 
METASOURCES = AUTO

lib_LTLIBRARIES = libktorrent.la
libktorrent_la_LDFLAGS = ${KDE_RPATH} $(all_libraries) -release $(VERSION)

kde_kcfg_DATA = ktorrent.kcfg

# make sure settings.h is built before anything else
BUILT_SOURCES=settings.h

noinst_HEADERS = expandablewidget.h functions.h ktversion.h labelview.h \
	pluginmanager.h pluginmanagerprefpage.h
libktorrent_la_SOURCES = expandablewidget.cpp functions.cpp labelview.cpp \
	labelviewitembase.ui pluginmanager.cpp pluginmanagerprefpage.cpp pluginmanagerwidget.ui \
	settings.kcfgc
libktorrent_la_LIBADD = ../libktorrent/net/libnet.la \
	../libktorrent/datachecker/libdatachecker.la ../libktorrent/mse/libmse.la ../libktorrent/migrate/libmigrate.la \
	../libktorrent/util/libutil.la ../libktorrent/torrent/libtorrent.la \
	../libktorrent/kademlia/libkademlia.la ../libktorrent/interfaces/libinterfaces.la $(LIB_KPARTS)
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
