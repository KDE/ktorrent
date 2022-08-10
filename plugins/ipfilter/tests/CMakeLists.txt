include(ECMAddTests)
ecm_add_test(ipblocklisttest.cpp ../ipblocklist.cpp TEST_NAME ipblocklisttest LINK_LIBRARIES ktcore Qt::Core Qt::Network KF5::Torrent  Qt::Test)
