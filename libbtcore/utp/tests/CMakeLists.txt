set(localwindowtest_SRCS localwindowtest.cpp)

kde4_add_unit_test(localwindowtest TESTNAME localwindowtest ${localwindowtest_SRCS})
target_link_libraries( localwindowtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)
