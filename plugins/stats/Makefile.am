INCLUDES = -I$(top_builddir)/libktorrent -I$(top_builddir)/ktorrent/libktorrent \
	-I$(srcdir)/../../libktorrent $(all_includes)
	
METASOURCES = AUTO
		
kde_module_LTLIBRARIES = ktstatsplugin.la		
		
ktstatsplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KPARTS) $(all_libraries)
ktstatsplugin_la_SOURCES = ChartDrawerData.cc ChartDrawer.cc statsspdwgt.ui statsconwgt.ui StatsSpd.cc StatsCon.cc sprefwgt.ui statspluginsettings.kcfgc \
		 StatsPluginPrefsPage.cc StatsPluginPrefs.cc statsplugin.cc

ktstatsplugin_la_LIBADD = $(LIB_QT) ../../libktorrent/libktorrent.la

noinst_HEADERS = ChartDrawerData.h ChartDrawer.h StatsSpd.h StatsCon.h StatsPluginPrefsPage.h StatsPluginPrefs.h statsplugin.h 

kde_services_DATA = ktstatsplugin.desktop
kde_kcfg_DATA = ktstatsplugin.kcfg
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
