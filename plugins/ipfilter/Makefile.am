INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktipfilterplugin.la
noinst_HEADERS = ipfilterplugin.h ipblockingprefpage.h antip2p.h \
	convertdialog.h
ktipfilterplugin_la_SOURCES = ipfilterplugin.cpp ipblockingpref.ui \
	ipblockingprefpage.cpp ipfilterpluginsettings.kcfgc antip2p.cpp convert_dlg.ui \
	convertdialog.cpp

# Libs needed by the plugin
ktipfilterplugin_la_LIBADD = $(LIB_KHTML) $(LIB_KPARTS) \
	../../libktorrent/libktorrent.la $(LIB_QT) \
	$(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)



# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktipfilterplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktsearchplugin
# plugins_DATA = ktsearchpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktipfilterplugin.desktop

kde_kcfg_DATA = ktipfilterplugin.kcfg

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
