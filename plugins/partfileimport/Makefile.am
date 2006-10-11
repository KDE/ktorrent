INCLUDES = -I$(top_builddir)/apps/ktorrent -I$(srcdir)/../../libktorrent \
	$(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktpartfileimportplugin.la
noinst_HEADERS = partfileimportplugin.h importdialog.h
ktpartfileimportplugin_la_SOURCES = partfileimportplugin.cpp importdlgbase.ui importdialog.cpp

# Libs needed by the plugin
ktpartfileimportplugin_la_LIBADD = $(LIB_KHTML) $(LIB_KPARTS) \
	../../libktorrent/libktorrent.la \
	$(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktpartfileimportplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
rcdir = $(kde_datadir)/ktorrent
rc_DATA = ktpartfileimportpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktpartfileimportplugin.desktop

# kde_kcfg_DATA =  ktpartfileimportplugin.kcfg

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
