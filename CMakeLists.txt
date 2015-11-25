cmake_minimum_required(VERSION 2.8.12)
project(KTORRENT)


set (QT_MIN_VERSION "5.5.0")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Widgets Script Sql)

find_package(KF5 REQUIRED KDELibs4Support I18n KIO XmlGui Notifications NotifyConfig Solid Config CoreAddons DocTools Kross Sonnet DBusAddons Crash)

find_package(LibKTorrent 1.9.50 CONFIG REQUIRED)
find_package(Boost REQUIRED)

#
# checks for functions and stuff
#
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckTypeSize) #XFS


# according to http://www.cmake.org/pipermail/cmake/2008-June/022381.html
kde_enable_exceptions()

#set (KTORRENT_DBUS_XML_DIR ${KTORRENT_SOURCE_DIR}/dbus_xml)

include_directories(
   ${KTORRENT_BINARY_DIR}
   ${KTORRENT_BINARY_DIR}/libktcore
   ${KTORRENT_SOURCE_DIR}/libktcore
   ${LIBKTORRENT_INCLUDE_DIR}
   ${KTORRENT_SOURCE_DIR}
   ${GMP_INCLUDE_DIR}
   ${Boost_INCLUDE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${QT_INCLUDES}
)

add_subdirectory(libktcore)
#add_subdirectory(plugins)
add_subdirectory(ktorrent)
#add_subdirectory(ktupnptest)
#add_subdirectory(plasma)
#add_subdirectory(ktmagnetdownloader)
#add_subdirectory(doc)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
