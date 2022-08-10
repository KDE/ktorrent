add_executable(ktmagnetdownloader)
ecm_mark_nongui_executable(ktmagnetdownloader)

target_sources(ktmagnetdownloader PRIVATE magnetdownloader.cpp magnettest.cpp)

target_link_libraries(ktmagnetdownloader ktcore Qt::Widgets Qt::Network)

install(TARGETS ktmagnetdownloader ${INSTALL_TARGETS_DEFAULT_ARGS})

