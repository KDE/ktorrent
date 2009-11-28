include_directories(${KTORRENT_SOURCE_DIR}/libktupnp)

set(magnetdownloadtest_SRCS magnetdownloader.cpp magnettest.cpp)
kde4_add_executable(ktmagnetdownloader ${magnetdownloadtest_SRCS})
target_link_libraries(ktmagnetdownloader ${QT_QTNETWORK_LIBRARY} ${KDE4_KDEUI_LIBS} ktcore ktupnp btcore)
install(TARGETS ktmagnetdownloader ${INSTALL_TARGETS_DEFAULT_ARGS})