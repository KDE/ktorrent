include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libktorrent)
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

kde4_add_ui_files(ktstatsplugin_SRC Spd.ui Conns.ui Settings.ui DisplaySettings.ui)
kde4_add_kcfg_files(ktstatsplugin_SRC statspluginsettings.kcfgc)
kde4_add_plugin(ktstatsplugin ${ktstatsplugin_SRC})

target_link_libraries(ktstatsplugin ${KDE4_KPARTS_LIBS} ktcore btcore)
install(TARGETS ktstatsplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktstatsplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
