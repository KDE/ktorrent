add_subdirectory(scripts)

set(ktscriptingplugin_SRC
	api/scriptingmodule.cpp
	api/scriptablegroup.cpp
	scriptingplugin.cpp 
	scriptmanager.cpp 
	scriptmodel.cpp
	scriptdelegate.cpp 
	script.cpp)

ki18n_wrap_ui(ktscriptingplugin_SRC scriptproperties.ui)
#kconfig_add_kcfg_files(ktscriptingplugin_SRC scriptingpluginsettings.kcfgc)
add_library(ktscriptingplugin MODULE ${ktscriptingplugin_SRC})

target_link_libraries(
    ktscriptingplugin
    ktcore
    ${LIBKTORRENT_LIBRARIES}
    KF5::KDELibs4Support
    KF5::KrossCore
    KF5::Parts
    Qt5::Core
    )
install(TARGETS ktscriptingplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktscriptingplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES ktscriptingpluginui.rc DESTINATION ${DATA_INSTALL_DIR}/ktorrent )
