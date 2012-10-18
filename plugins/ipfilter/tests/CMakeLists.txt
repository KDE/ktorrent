set(ipblocklisttest_SRCS ipblocklisttest.cpp ../ipblocklist.cpp)
kde4_add_unit_test(ipblocklisttest TESTNAME ipblocklisttest ${ipblocklisttest_SRCS})
target_link_libraries(ipblocklisttest ${QT_QTTEST_LIBRARY} ktorrent)
