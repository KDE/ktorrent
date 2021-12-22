include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/drawer)

ktorrent_add_plugin(ktorrent_stats)

target_sources(ktorrent_stats PRIVATE
	drawer/ChartDrawer.cc
	drawer/ChartDrawerData.cc
	drawer/ChartDrawerData.h
	drawer/ChartDrawer.h
	drawer/PlainChartDrawer.cc
	drawer/PlainChartDrawer.h
	drawer/KPlotWgtDrawer.cc
	drawer/KPlotWgtDrawer.h

	StatsPlugin.cc 
	PluginPage.cc 
	SpdTabPage.cc 
	ConnsTabPage.cc
	SettingsPage.cc
	DisplaySettingsPage.cc
)

ki18n_wrap_ui(ktorrent_stats Spd.ui Conns.ui Settings.ui DisplaySettings.ui)
kconfig_add_kcfg_files(ktorrent_stats statspluginsettings.kcfgc)
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
