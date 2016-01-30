set(magnetdownloadtest_SRCS magnetdownloader.cpp magnettest.cpp)
add_executable(ktmagnetdownloader ${magnetdownloadtest_SRCS})
target_link_libraries(ktmagnetdownloader Qt5::Network  ktcore ktorrent)
install(TARGETS ktmagnetdownloader ${INSTALL_TARGETS_DEFAULT_ARGS})

