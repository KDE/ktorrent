cmake_minimum_required(VERSION 2.8.12)
project(KTORRENT)


set (QT_MIN_VERSION "5.5.0")
set (KF5_MIN_VERSION "5.15")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMInstallIcons)
include(ECMMarkAsTest)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED
    Core
    DBus
    Network
    Script
    Widgets
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    I18n
    IconThemes
    KIO
    Notifications
    NotifyConfig
    KCMUtils
    Parts
    Service
    Solid
    WidgetsAddons
    WindowSystem
    XmlGui
)

find_package(KF5DocTools ${KF5_MIN_VERSION})

find_package(KF5Torrent 1.9.50 CONFIG REQUIRED)
find_package(Boost REQUIRED)

#
# checks for functions and stuff
#
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckTypeSize) #XFS


# according to http://www.cmake.org/pipermail/cmake/2008-June/022381.html
kde_enable_exceptions()

add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_CAST_TO_BYTEARRAY
#   -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
)

set (KTORRENT_DBUS_XML_DIR ${KTORRENT_SOURCE_DIR}/dbus_xml)
set (KTORRENT_PLUGIN_INSTALL_DIR ${PLUGIN_INSTALL_DIR}/ktorrent)

include_directories(
   ${Boost_INCLUDE_DIR}
)



add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_NO_URL_CAST_FROM_STRING -DQT_NO_CAST_TO_ASCII)

add_subdirectory(libktcore)
add_subdirectory(plugins)
add_subdirectory(ktorrent)
add_subdirectory(ktupnptest)
#add_subdirectory(plasma)
add_subdirectory(ktmagnetdownloader)
if (KF5DocTools_FOUND)
    add_subdirectory(doc)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
