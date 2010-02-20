set(polltest_SRCS polltest.cpp)
kde4_add_unit_test(polltest TESTNAME polltest ${polltest_SRCS})
target_link_libraries( polltest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(wakeuppipetest_SRCS wakeuppipetest.cpp)
kde4_add_unit_test(wakeuppipetest TESTNAME wakeuppipetest ${wakeuppipetest_SRCS})
target_link_libraries( wakeuppipetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)