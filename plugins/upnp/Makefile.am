INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO

libktupnp_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libktupnp.la
libktupnp_la_SOURCES = soap.cpp upnpdescriptionparser.cpp upnpmcastsocket.cpp\
	upnprouter.cpp

kde_module_LTLIBRARIES = ktupnpplugin.la
noinst_HEADERS = upnpplugin.h upnpmcastsocket.h upnprouter.h upnpprefpage.h \
		upnpprefwidget.h upnpdescriptionparser.h soap.h
ktupnpplugin_la_SOURCES = upnpplugin.cpp upnpprefpage.cpp upnpwidget.ui \
	upnpprefwidget.cpp upnppluginsettings.kcfgc

# Libs needed by the plugin
ktupnpplugin_la_LIBADD = libktupnp.la \
	$(LIB_KPARTS) ../../libktorrent/libktorrent.la \
	$(LIB_QT) \
	$(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)



# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktupnpplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktsearchplugin
# plugins_DATA = ktsearchpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktupnpplugin.desktop

kde_kcfg_DATA = ktupnpplugin.kcfg


KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
