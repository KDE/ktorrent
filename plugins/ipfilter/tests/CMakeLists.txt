include(ECMAddTests)
ecm_add_test(ipblocklisttest.cpp ../ipblocklist.cpp TEST_NAME ipblocklisttest LINK_LIBRARIES ktcore Qt5::Core Qt5::Network KF5::Torrent  Qt5::Test)
