set(localwindowtest_SRCS localwindowtest.cpp)
kde4_add_unit_test(localwindowtest TESTNAME localwindowtest ${localwindowtest_SRCS})
target_link_libraries( localwindowtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(connectiontest_SRCS connectiontest.cpp)
kde4_add_unit_test(connectiontest TESTNAME connectiontest ${connectiontest_SRCS})
target_link_libraries( connectiontest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(sendtest_SRCS sendtest.cpp)
kde4_add_unit_test(sendtest TESTNAME sendtest ${sendtest_SRCS})
target_link_libraries( sendtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)