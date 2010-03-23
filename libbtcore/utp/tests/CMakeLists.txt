set(localwindowtest_SRCS localwindowtest.cpp)
kde4_add_unit_test(localwindowtest TESTNAME localwindowtest ${localwindowtest_SRCS})
target_link_libraries( localwindowtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(remotewindowtest_SRCS remotewindowtest.cpp)
kde4_add_unit_test(remotewindowtest TESTNAME remotewindowtest ${remotewindowtest_SRCS})
target_link_libraries( remotewindowtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(connectiontest_SRCS connectiontest.cpp)
kde4_add_unit_test(connectiontest TESTNAME connectiontest ${connectiontest_SRCS})
target_link_libraries( connectiontest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(sendtest_SRCS sendtest.cpp)
kde4_add_unit_test(sendtest TESTNAME sendtest ${sendtest_SRCS})
target_link_libraries( sendtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(connecttest_SRCS connecttest.cpp)
kde4_add_unit_test(connecttest TESTNAME connecttest ${connecttest_SRCS})
target_link_libraries( connecttest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(transmittest_SRCS transmittest.cpp)
kde4_add_unit_test(transmittest TESTNAME transmittest ${transmittest_SRCS})
target_link_libraries( transmittest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(packetlosstest_SRCS packetlosstest.cpp)
kde4_add_unit_test(packetlosstest TESTNAME packetlosstest ${packetlosstest_SRCS})
target_link_libraries( packetlosstest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(fintest_SRCS fintest.cpp)
kde4_add_unit_test(fintest TESTNAME fintest ${fintest_SRCS})
target_link_libraries( fintest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(sockettest_SRCS sockettest.cpp)
kde4_add_unit_test(sockettest TESTNAME sockettest ${sockettest_SRCS})
target_link_libraries( sockettest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(utppolltest_SRCS utppolltest.cpp)
kde4_add_unit_test(utppolltest TESTNAME utppolltest ${utppolltest_SRCS})
target_link_libraries( utppolltest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)

set(delaywindowtest_SRCS delaywindowtest.cpp)
kde4_add_unit_test(delaywindowtest TESTNAME delaywindowtest ${delaywindowtest_SRCS})
target_link_libraries( delaywindowtest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} btcore)