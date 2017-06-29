set(ktupnptest_SRC main.cpp upnptestwidget.cpp)

ki18n_wrap_ui(ktupnptest_SRC upnptestwidget.ui)
add_executable(ktupnptest ${ktupnptest_SRC})

target_link_libraries(
    ktupnptest
    Qt5::Network
    KF5::Torrent
    KF5::CoreAddons
    KF5::I18n
)
install(TARGETS ktupnptest ${INSTALL_TARGETS_DEFAULT_ARGS})

