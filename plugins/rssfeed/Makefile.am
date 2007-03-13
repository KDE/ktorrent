INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktrssfeedplugin.la


# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktrssfeedplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktrssfeedplugin
# plugins_DATA = ktrssfeedpluginui.rc

# Install the desktop file needed to detect the plugin

rcdir = $(kde_datadir)/ktorrent

kde_kcfg_DATA = ktrssfeedplugin.kcfg
kde_services_DATA = ktrssfeedplugin.desktop

noinst_HEADERS = rssfeedplugin.h rssfeedmanager.h rssfeed.h rssfilter.h \
		rssarticle.h rsslinkdownloader.h
ktrssfeedplugin_la_SOURCES = rssfeedplugin.cpp rssfeedmanager.cpp \
			rssfeedwidget.ui rssfeed.cpp rssfilter.cpp rssarticle.cpp rsslinkdownloader.cpp
ktrssfeedplugin_la_LIBADD = $(LIB_KIO) $(LIB_QT) rss/librsslocal.la \
	../../libktorrent/libktorrent.la $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KHTML) $(LIB_KPARTS)

SUBDIRS = rss
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
