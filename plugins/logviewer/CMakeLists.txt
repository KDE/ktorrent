include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libktorrent)

set(ktlogviewerplugin_SRC logviewerplugin.cpp logflags.cpp logviewer.cpp logprefpage.cpp)

kde4_add_ui_files(ktlogviewerplugin_SRC logprefwidget.ui)
kde4_add_kcfg_files(ktlogviewerplugin_SRC logviewerpluginsettings.kcfgc)
kde4_add_plugin(ktlogviewerplugin ${ktlogviewerplugin_SRC})

target_link_libraries(ktlogviewerplugin ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} -lkparts ktcore )
install(TARGETS ktlogviewerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES ktlogviewerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

