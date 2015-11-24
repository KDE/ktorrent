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
add_library(ktstatsplugin MODULE ${ktstatsplugin_SRC})

target_link_libraries(
    ktstatsplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    KF5::Parts
    )
install(TARGETS ktstatsplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktstatsplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
