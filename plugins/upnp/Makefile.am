INCLUDES = $(all_includes) -I$(srcdir)/../../libktorrent
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktupnpplugin.la
noinst_HEADERS = upnpplugin.h upnpmcastsocket.h upnprouter.h upnpprefpage.h \
		upnpprefwidget.h upnpdescriptionparser.h soap.h
ktupnpplugin_la_SOURCES = upnpplugin.cpp upnpmcastsocket.cpp upnprouter.cpp \
		upnpprefpage.cpp upnpwidget.ui upnpprefwidget.cpp upnpdescriptionparser.cpp soap.cpp upnppluginsettings.kcfgc

# Libs needed by the plugin
ktupnpplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KPARTS) \
	../../libktorrent/libktorrent.la

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

