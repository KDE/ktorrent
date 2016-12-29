
# [Fonic] https://community.kde.org/Frameworks/Porting_Notes#Build_System
include(ECMAddTests)

# [Fonic] http://stackoverflow.com/questions/4753845/build-qt-tests-with-cmake
find_package(Qt5Test REQUIRED)

set(ipblocklisttest_SRCS ipblocklisttest.cpp ../ipblocklist.cpp)
# [Fonic] 'ipblocklisttestNAME' -> typo, copy/paste error?
#add_executable(ipblocklisttestNAME ipblocklisttest ${ipblocklisttest_SRCS})
add_executable(ipblocklisttest ${ipblocklisttest_SRCS})
add_test(ipblocklisttest ipblocklisttest)
ecm_mark_as_test(ipblocklisttest)
# [Fonic] Missing second ':' in 'KF5:Torrent' -> most likely typo
#target_link_libraries(ipblocklisttest Qt5::Core Qt5::Network Qt5::Test KF5:Torrent)
target_link_libraries(ipblocklisttest Qt5::Core Qt5::Network Qt5::Test KF5::Torrent)
