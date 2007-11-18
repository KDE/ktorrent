INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = ktinfowidgetplugin.la
noinst_HEADERS = infowidgetplugin.h infowidgetprefpage.h trackerview.h GeoIP.h \
	statustab.h fileview.h 
ktinfowidgetplugin_la_SOURCES = infowidgetplugin.cpp availabilitychunkbar.cpp \
	chunkbar.cpp chunkdownloadview.cpp downloadedchunkbar.cpp flagdb.cpp peerview.cpp \
	ktorrentmonitor.cpp iwfiletreediritem.cpp iwfiletreeitem.cpp infowidgetprefpage.cpp \
	infowidgetpluginsettings.kcfgc iwpref.ui trackerviewbase.ui trackerview.cpp floatspinbox.cpp \
	localefloatvalidator.cpp chunkdownloadviewbase.ui statustabbase.ui statustab.cpp fileview.cpp 

# Libs needed by the plugin
ktinfowidgetplugin_la_LIBADD = ../../libktorrent/libktorrent.la \
			   $(LIB_KHTML) $(LIB_KPARTS) $(LIB_QT) \
			   $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)


# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
ktinfowidgetplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
if USE_SYSTEM_GEOIP
ktinfowidgetplugin_la_LDFLAGS += -lGeoIP
else
ktinfowidgetplugin_la_SOURCES += GeoIP.c
endif

# rc file containing the GUI for the plugin
# pluginsdir = $(kde_datadir)/ktsearchplugin
# plugins_DATA = ktsearchpluginui.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = ktinfowidgetplugin.desktop

kde_kcfg_DATA = ktinfowidgetplugin.kcfg

ktdatadir = $(kde_datadir)/ktorrent/geoip

SUBDIRS = geoip
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
