set(ktmagnetgeneratorplugin_SRC 
	magnetgeneratorprefwidget.cpp
	magnetgeneratorplugin.cpp)

ki18n_wrap_ui(ktmagnetgeneratorplugin_SRC magnetgeneratorprefwidget.ui)
kconfig_add_kcfg_files(ktmagnetgeneratorplugin_SRC magnetgeneratorpluginsettings.kcfgc)
add_library(ktorrent_magnetgenerator MODULE ${ktmagnetgeneratorplugin_SRC})

kcoreaddons_desktop_to_json(ktorrent_magnetgenerator ktorrent_magnetgenerator.desktop)

target_link_libraries(
    ktorrent_magnetgenerator
    ktcore
    KF5::Torrent
    KF5::CoreAddons
    KF5::I18n
    KF5::Notifications
    KF5::XmlGui
)

target_include_directories(ktorrent_magnetgenerator PRIVATE "$<BUILD_INTERFACE:${KTORRENT_SOURCE_DIR}>")

install(TARGETS ktorrent_magnetgenerator  DESTINATION ${KTORRENT_PLUGIN_INSTALL_DIR} )
install(FILES ktorrent_magnetgeneratorui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/ktorrent )
