set(magnetlinktest_SRCS magnetlinktest.cpp)

kde4_add_unit_test(magnetlinktest TESTNAME magnetlinktest ${magnetlinktest_SRCS})
target_link_libraries( magnetlinktest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)