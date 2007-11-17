include_directories(${KTORRENT_SOURCE_DIR}/libktupnp)

set(ktupnptest_SRC main.cpp upnptestwidget.cpp)
	

kde4_add_ui_files(ktupnptest_SRC upnptestwidget.ui)
kde4_add_executable(ktupnptest ${ktupnptest_SRC})

target_link_libraries(ktupnptest btcore ktupnp ${KDE4_KDEUI_LIBS})
install(TARGETS ktupnptest DESTINATION ${BIN_INSTALL_DIR})

