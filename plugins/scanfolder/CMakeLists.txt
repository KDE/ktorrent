set(ktscanfolderplugin_SRC scanfolder.cpp scanfolderplugin.cpp scanfolderprefpage.cpp)

kde4_add_ui_files(ktscanfolderplugin_SRC scanfolderprefpage.ui)
kde4_add_kcfg_files(ktscanfolderplugin_SRC scanfolderpluginsettings.kcfgc)
kde4_add_plugin(ktscanfolderplugin ${ktscanfolderplugin_SRC})

target_link_libraries(ktscanfolderplugin ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} -lkparts ktcore )
install(TARGETS ktscanfolderplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktscanfolderplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

