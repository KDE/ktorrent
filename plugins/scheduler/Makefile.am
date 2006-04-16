INCLUDES = $(all_includes) -I$(srcdir)/../../libktorrent
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktschedulerplugin.la
noinst_HEADERS = schedulerplugin.h bwsprefpagewidget.h bwsprefpage.h \
	bwscheduler.h
ktschedulerplugin_la_SOURCES = schedulerplugin.cpp \
	schedulerpluginsettings.kcfgc bwspage.ui bwsprefpagewidget.cpp bwsprefpage.cpp bwscheduler.cpp

# Libs needed by the plugin
ktschedulerplugin_la_LIBADD = $(LIB_KHTML) $(LIB_KPARTS) \
	../../libktorrent/libktorrent.la

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktschedulerplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktschedulerplugin
# plugins_DATA = ktschedulerpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktschedulerplugin.desktop

kde_kcfg_DATA = ktschedulerplugin.kcfg

