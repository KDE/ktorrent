set(ktupnptest_SRC main.cpp upnptestwidget.cpp)
	

ki18n_wrap_ui(ktupnptest_SRC upnptestwidget.ui)
add_executable(ktupnptest ${ktupnptest_SRC})

target_link_libraries(ktupnptest ktorrent Qt5::Network)
install(TARGETS ktupnptest ${INSTALL_TARGETS_DEFAULT_ARGS})

