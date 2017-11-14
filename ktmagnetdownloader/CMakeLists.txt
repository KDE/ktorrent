set(magnetdownloadtest_SRCS magnetdownloader.cpp magnettest.cpp)
add_executable(ktmagnetdownloader ${magnetdownloadtest_SRCS})
target_link_libraries(ktmagnetdownloader ktcore Qt5::Widgets Qt5::Network)
ecm_mark_nongui_executable(ktmagnetdownloader)
install(TARGETS ktmagnetdownloader ${INSTALL_TARGETS_DEFAULT_ARGS})

