include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/drawer)

set(Drawer_SRC
	drawer/ChartDrawer.cc
	drawer/ChartDrawerData.cc
	drawer/ChartDrawerData.h
	drawer/ChartDrawer.h
	drawer/PlainChartDrawer.cc
	drawer/PlainChartDrawer.h
	drawer/KPlotWgtDrawer.cc
	drawer/KPlotWgtDrawer.h
)

set(ktstatsplugin_SRC 
	${Drawer_SRC} 
	StatsPlugin.cc 
	PluginPage.cc 
	SpdTabPage.cc 
	ConnsTabPage.cc
	SettingsPage.cc
	DisplaySettingsPage.cc
)

ki18n_wrap_ui(ktstatsplugin_SRC Spd.ui Conns.ui Settings.ui DisplaySettings.ui)
kconfig_add_kcfg_files(ktstatsplugin_SRC statspluginsettings.kcfgc)
add_library(ktorrent_stats MODULE ${ktstatsplugin_SRC})
kcoreaddons_desktop_to_json(ktorrent_stats ktorrent_stats.desktop)

target_link_libraries(
    ktorrent_stats
    ktcore
    KF5::Torrent
    KF5::CoreAddons
    KF5::I18n
    KF5::Plotting
    KF5::WidgetsAddons
)
install(TARGETS ktorrent_stats  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
