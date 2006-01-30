INCLUDES = $(all_includes) -I$(srcdir)/../../libktorrent

METASOURCES = AUTO
kde_module_LTLIBRARIES = ktlogviewerplugin.la


# Libs needed by the plugin
ktlogviewerplugin_la_LIBADD = $(LIB_KHTML) $(LIB_KPARTS) \
	../../libktorrent/libktorrent.la

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktlogviewerplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktsearchplugin
# plugins_DATA = ktsearchpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktlogviewerplugin.desktop

noinst_HEADERS = logviewerplugin.h logviewer.h
ktlogviewerplugin_la_SOURCES = logviewerplugin.cpp logviewer.cpp
