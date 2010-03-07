set(statsfiletest_SRCS statsfiletest.cpp)
kde4_add_unit_test(statsfiletest TESTNAME statsfiletest ${statsfiletest_SRCS})
target_link_libraries( statsfiletest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)