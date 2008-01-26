INCLUDES = -I$(top_builddir)/libktorrent -I$(top_builddir)/ktorrent/libktorrent \
	-I$(srcdir)/../../libktorrent $(all_includes)
	
METASOURCES = AUTO
		
if COMPILE_ZEROCONF
kde_module_LTLIBRARIES = ktzeroconfplugin.la		
endif
		
ktzeroconfplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(AVAHI_LIBS) $(AVAHI_QT3_LIBS)
ktzeroconfplugin_la_SOURCES = localbrowser.cpp avahiservice.cpp \
	zeroconfplugin.cpp

ktzeroconfplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KDEUI)\
	$(LIB_QT) ../../libktorrent/libktorrent.la

noinst_HEADERS = zeroconfplugin.h

kde_services_DATA = ktzeroconfplugin.desktop
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)


