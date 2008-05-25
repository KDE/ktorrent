INCLUDES = -I$(top_builddir)/libktorrent -I$(top_builddir)/ktorrent/libktorrent \
	-I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktwebinterfaceplugin.la
noinst_HEADERS = webinterfaceplugin.h httpserver.h php_handler.h \
		php_interface.h webinterfaceprefwidget.h		webinterfaceprefpage.h httpclienthandler.h \
	httpresponseheader.h
ktwebinterfaceplugin_la_SOURCES = webinterfaceplugin.cpp httpserver.cpp \
		php_handler.cpp php_interface.cpp				webinterfacepref.ui \
		webinterfacepluginsettings.kcfgc webinterfaceprefwidget.cpp				webinterfaceprefpage.cpp \
		httpclienthandler.cpp httpresponseheader.cpp
# Libs needed by the plugin
ktwebinterfaceplugin_la_LIBADD = ../../libktorrent/libktorrent.la \
			   $(LIB_KHTML) $(LIB_KPARTS) $(LIB_QT) \
			   $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktwebinterfaceplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktsearchplugin
# plugins_DATA = ktsearchpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktwebinterfaceplugin.desktop

kde_kcfg_DATA = ktwebinterfaceplugin.kcfg

ktdatadir = $(kde_datadir)/ktorrent/www

SUBDIRS = www
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
