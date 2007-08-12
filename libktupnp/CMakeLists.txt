include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libktorrent)
include_directories(${CMAKE_BINARY_DIR}/libktorrent)


set(libktupnp_SRC httprequest.cpp soap.cpp upnpdescriptionparser.cpp upnpmcastsocket.cpp upnprouter.cpp)

kde4_add_library(ktupnp SHARED ${libktupnp_SRC})

target_link_libraries(ktupnp ktcore)
set_target_properties(ktupnp PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS ktupnp  DESTINATION ${LIB_INSTALL_DIR} )

