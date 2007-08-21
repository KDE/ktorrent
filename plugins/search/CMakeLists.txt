set(ktsearchplugin_SRC searchenginelist.cpp htmlpart.cpp searchprefpage.cpp searchwidget.cpp searchtoolbar.cpp searchplugin.cpp)

kde4_add_ui_files(ktsearchplugin_SRC searchpref.ui searchbar.ui)
kde4_add_kcfg_files(ktsearchplugin_SRC searchpluginsettings.kcfgc)
kde4_add_plugin(ktsearchplugin ${ktsearchplugin_SRC})

target_link_libraries(ktsearchplugin ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} -lkparts -lkhtml ktcore )
install(TARGETS ktsearchplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktsearchplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

