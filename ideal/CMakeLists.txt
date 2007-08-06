include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(ktideal_SRC box.cpp mainwindow.cpp sidebar.cpp)
kde4_add_library(ktideal STATIC ${ktideal_SRC})

target_link_libraries(ktideal ${KDE4_KDEUI_LIBS})
# install(TARGETS ktideal  DESTINATION ${LIB_INSTALL_DIR} )

