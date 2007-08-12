include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libktorrent)
include_directories(${CMAKE_SOURCE_DIR}/libktupnp)
include_directories(${CMAKE_BINARY_DIR}/libktorrent)

set(ktupnptest_SRC main.cpp upnptestwidget.cpp)
	

kde4_add_ui_files(ktupnptest_SRC upnptestwidget.ui)
kde4_add_executable(ktupnptest ${ktupnptest_SRC})

target_link_libraries(ktupnptest ktcore ktupnp ${KDE4_KDEUI_LIBS})
install(TARGETS ktupnptest DESTINATION ${BIN_INSTALL_DIR})

