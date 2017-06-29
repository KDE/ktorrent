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

add_library(ktorrent_scripting MODULE ${ktscriptingplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_scripting ktorrent_scripting.desktop)

target_link_libraries(
    ktorrent_scripting
    ktcore
    Qt5::Core
    KF5::Torrent
    KF5::Archive
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::ItemViews
    KF5::KIOWidgets
    KF5::KrossCore
    KF5::XmlGui
)

install(TARGETS ktorrent_scripting  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_scriptingui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
