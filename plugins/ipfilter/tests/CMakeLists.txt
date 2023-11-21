include(ECMAddTests)
ecm_add_test(ipblocklisttest.cpp ../ipblocklist.cpp TEST_NAME ipblocklisttest LINK_LIBRARIES ktcore Qt6::Core Qt6::Network KTorrent6  Qt6::Test)
