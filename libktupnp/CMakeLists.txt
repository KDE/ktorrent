
set(libktupnp_SRC httprequest.cpp soap.cpp upnpdescriptionparser.cpp upnpmcastsocket.cpp upnprouter.cpp)

kde4_add_library(ktupnp SHARED ${libktupnp_SRC})

target_link_libraries(
    ktupnp
    ktcore 
    ${LIBKTORRENT_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY})
set_target_properties(ktupnp PROPERTIES VERSION 4.0.2 SOVERSION 4 )
install(TARGETS ktupnp  ${INSTALL_TARGETS_DEFAULT_ARGS} )

