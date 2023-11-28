include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/drawer)

ktorrent_add_plugin(StatsPlugin)

target_sources(StatsPlugin PRIVATE
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

ki18n_wrap_ui(StatsPlugin Spd.ui Conns.ui Settings.ui DisplaySettings.ui)
kconfig_add_kcfg_files(StatsPlugin statspluginsettings.kcfgc)

target_link_libraries(
    StatsPlugin
    ktcore
    KTorrent6
    KF6::CoreAddons
    KF6::I18n
    KF6::Plotting
    KF6::WidgetsAddons
)
