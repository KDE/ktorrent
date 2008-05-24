set(ktbwschedulerplugin_SRC bwschedulerplugin.cpp weekview.cpp weekscene.cpp schedule.cpp scheduleeditor.cpp additemdlg.cpp schedulegraphicsitem.cpp bwprefpage.cpp)

kde4_add_ui_files(ktbwschedulerplugin_SRC additemdlg.ui bwprefpage.ui)
kde4_add_kcfg_files(ktbwschedulerplugin_SRC bwschedulerpluginsettings.kcfgc)
kde4_add_plugin(ktbwschedulerplugin ${ktbwschedulerplugin_SRC})

target_link_libraries(ktbwschedulerplugin ${QT_QT3SUPPORT_LIBRARY}
${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ktcore )
install(TARGETS ktbwschedulerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktbwschedulerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

