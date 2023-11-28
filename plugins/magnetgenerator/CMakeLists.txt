ktorrent_add_plugin(MagnetGeneratorPlugin)

target_sources(MagnetGeneratorPlugin PRIVATE
	magnetgeneratorprefwidget.cpp
	magnetgeneratorplugin.cpp)

ki18n_wrap_ui(MagnetGeneratorPlugin magnetgeneratorprefwidget.ui)
kconfig_add_kcfg_files(MagnetGeneratorPlugin magnetgeneratorpluginsettings.kcfgc)

target_link_libraries(
    MagnetGeneratorPlugin
    ktcore
    KTorrent6
    KF6::CoreAddons
    KF6::I18n
    KF6::Notifications
    KF6::XmlGui
)

target_include_directories(MagnetGeneratorPlugin PRIVATE "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>")

install(FILES ktorrent_magnetgeneratorui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/ktorrent )
