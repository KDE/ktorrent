INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktscanfolderplugin.la


# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktscanfolderplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktscanfolderplugin
# plugins_DATA = ktscanfolderpluginui.rc

# Install the desktop file needed to detect the plugin

rcdir = $(kde_datadir)/ktorrent

kde_kcfg_DATA = ktscanfolderplugin.kcfg
kde_services_DATA = ktscanfolderplugin.desktop

noinst_HEADERS = scanfolderplugin.h scanfolderprefpage.h \
	scanfolderprefpagewidget.h scanfolder.h
ktscanfolderplugin_la_SOURCES = scanfolderplugin.cpp \
	scanfolderpluginsettings.kcfgc scanfolderprefpage.cpp sfprefwidgetbase.ui scanfolderprefpagewidget.cpp \
	scanfolder.cpp
ktscanfolderplugin_la_LIBADD = $(LIB_KPARTS) ../../libktorrent/libktorrent.la \
		       $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)


KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
