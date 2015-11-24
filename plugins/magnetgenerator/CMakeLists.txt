set(ktmagnetgeneratorplugin_SRC 
	magnetgeneratorprefwidget.cpp
	magnetgeneratorplugin.cpp)

ki18n_wrap_ui(ktmagnetgeneratorplugin_SRC magnetgeneratorprefwidget.ui)
kconfig_add_kcfg_files(ktmagnetgeneratorplugin_SRC magnetgeneratorpluginsettings.kcfgc)
add_library(ktmagnetgeneratorplugin MODULE ${ktmagnetgeneratorplugin_SRC})

target_link_libraries(
    ktmagnetgeneratorplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    KF5::Parts
    )
install(TARGETS ktmagnetgeneratorplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktmagnetgeneratorplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktmagnetgeneratorpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
