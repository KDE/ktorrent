set(ktengine_SRCS
    engine.cpp coredbusinterface.cpp torrentdbusinterface.cpp
)

add_library(plasma_engine_ktorrent MODULE ${ktengine_SRCS})
target_link_libraries(plasma_engine_ktorrent KF5::KDELibs4Support ${KDE4_PLASMA_LIBS} ${LIBKTORRENT_LIBRARIES})

install(TARGETS plasma_engine_ktorrent DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-ktorrent.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
