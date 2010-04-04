set(ktengine_SRCS
    engine.cpp coredbusinterface.cpp torrentdbusinterface.cpp
)

kde4_add_plugin(plasma_engine_ktorrent ${ktengine_SRCS})
target_link_libraries(plasma_engine_ktorrent ${KDE4_KDECORE_LIBS} ${PLASMA_LIBS} ${LIBKTORRENT_LIBRARIES})

install(TARGETS plasma_engine_ktorrent DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-ktorrent.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
