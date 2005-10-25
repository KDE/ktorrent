INCLUDES = $(all_includes) -I$(srcdir)/../../libktorrent
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktinfowidgetplugin.la
noinst_HEADERS = infowidgetplugin.h infowidgetprefpage.h
ktinfowidgetplugin_la_SOURCES = infowidgetplugin.cpp availabilitychunkbar.cpp \
					chunkbar.cpp chunkdownloadview.cpp downloadedchunkbar.cpp infowidget.cpp \
					infowidgetbase.ui peerview.cpp ktorrentmonitor.cpp iwfiletreediritem.cpp \
			iwfiletreeitem.cpp infowidgetprefpage.cpp infowidgetpluginsettings.kcfgc iwpref.ui

# Libs needed by the plugin
ktinfowidgetplugin_la_LIBADD = $(LIB_KHTML) $(LIB_KPARTS) \
	../../libktorrent/libktorrent.la

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktinfowidgetplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktsearchplugin
# plugins_DATA = ktsearchpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktinfowidgetplugin.desktop

kde_kcfg_DATA = ktinfowidgetplugin.kcfg

