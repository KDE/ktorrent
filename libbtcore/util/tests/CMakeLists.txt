set(circularbuffertest_SRCS circularbuffertest.cpp)
kde4_add_unit_test(circularbuffertest TESTNAME circularbuffertest ${circularbuffertest_SRCS})
target_link_libraries( circularbuffertest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)