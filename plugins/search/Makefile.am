INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktsearchplugin.la
noinst_HEADERS = searchplugin.h searchprefpage.h searchtab.h searchenginelist.h
ktsearchplugin_la_SOURCES = searchplugin.cpp htmlpart.cpp searchbar.ui \
	searchpref.ui searchwidget.cpp searchprefpage.cpp searchpluginsettings.kcfgc \
	searchtab.cpp searchenginelist.cpp

# Libs needed by the plugin
ktsearchplugin_la_LIBADD = ../../libktorrent/libktorrent.la \
			   $(LIB_KHTML) $(LIB_KPARTS) $(LIB_QT) \
			   $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktsearchplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktsearchplugin
# plugins_DATA = ktsearchpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktsearchplugin.desktop

kde_kcfg_DATA = ktsearchplugin.kcfg

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
