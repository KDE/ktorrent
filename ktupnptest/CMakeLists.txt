add_executable(ktupnptest)

target_sources(ktupnptest PRIVATE main.cpp upnptestwidget.cpp)

ki18n_wrap_ui(ktupnptest upnptestwidget.ui)

target_link_libraries(
    ktupnptest
    Qt5::Network
    KF5::Torrent
    KF5::CoreAddons
    KF5::I18n
)
install(TARGETS ktupnptest ${INSTALL_TARGETS_DEFAULT_ARGS})

