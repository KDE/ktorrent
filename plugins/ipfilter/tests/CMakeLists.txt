include(ECMAddTests)
ecm_add_test(ipblocklisttest.cpp ../ipblocklist.cpp TEST_NAME ipblocklisttest LINK_LIBRARIES ktcore Qt6::Core Qt6::Network KF6::Torrent  Qt6::Test)
