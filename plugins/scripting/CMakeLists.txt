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
add_library(ktorrent_scripting MODULE ${ktscriptingplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_scripting ktorrent_scripting.desktop)

target_link_libraries(
    ktorrent_scripting
    ktcore
    KF5::Torrent
    KF5::KDELibs4Support
    KF5::KrossCore
    KF5::Parts
    Qt5::Core
    )
install(TARGETS ktorrent_scripting  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_scriptingui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
