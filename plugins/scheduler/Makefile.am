INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktschedulerplugin.la
noinst_HEADERS = schedulerplugin.h bwscheduler.h schedulerprefpagewidget.h \
	bwsprefpagewidget.h bwswidget.h schedulerprefpage.h
ktschedulerplugin_la_SOURCES = schedulerplugin.cpp \
	schedulerpluginsettings.kcfgc bwscheduler.cpp schedulerprefpagewidget.cpp schedulerpage.ui bwspage.ui \
	bwsprefpagewidget.cpp bwswidget.cpp schedulerprefpage.cpp

# Libs needed by the plugin
ktschedulerplugin_la_LIBADD = $(LIB_KHTML) $(LIB_KPARTS) \
	../../libktorrent/libktorrent.la \
	$(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)

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

kticonsdir = $(kde_datadir)/ktorrent/icons

kticons_DATA = cell-a-0000.png cell-a-0001.png cell-a-0002.png cell-a-0003.png\
	cell-a-0004.png cell-b-0000.png cell-b-0001.png cell-b-0002.png\
	cell-b-0003.png cell-b-0004.png
rcdir = $(kde_datadir)/ktorrent
rc_DATA = ktschedulerpluginui.rc
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
