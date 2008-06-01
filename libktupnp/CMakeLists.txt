include_directories(${KTORRENT_SOURCE_DIR}/libktorrent)
include_directories(${CMAKE_KTORRENT_BINARY_DIR}/libktorrent)

set(libktupnp_SRC httprequest.cpp soap.cpp upnpdescriptionparser.cpp upnpmcastsocket.cpp upnprouter.cpp)

kde4_add_library(ktupnp SHARED ${libktupnp_SRC})

target_link_libraries(ktupnp ktcore)
set_target_properties(ktupnp PROPERTIES VERSION 3.0.0 SOVERSION 3 )
install(TARGETS ktupnp  ${INSTALL_TARGETS_DEFAULT_ARGS} )

