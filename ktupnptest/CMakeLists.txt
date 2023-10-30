add_executable(ktupnptest)

target_sources(ktupnptest PRIVATE main.cpp upnptestwidget.cpp)

ki18n_wrap_ui(ktupnptest upnptestwidget.ui)

target_link_libraries(
    ktupnptest
    Qt::Network
    KF5::Torrent
    KF6::CoreAddons
    KF6::I18n
)
install(TARGETS ktupnptest ${INSTALL_TARGETS_DEFAULT_ARGS})

