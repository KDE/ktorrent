INCLUDES = $(all_includes) -I$(srcdir)/../../libktorrent
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktsearchplugin.la
noinst_HEADERS = searchplugin.h
ktsearchplugin_la_SOURCES = searchplugin.cpp

# Libs needed by the plugin
ktsearchplugin_la_LIBADD = $(LIB_KPARTS) $(top_builddir)/libktorrent/interfaces/libinterfaces.la

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktsearchplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktsearchplugin
# plugins_DATA = ktsearchpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktsearchplugin.desktop
