cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "24")
set (RELEASE_SERVICE_VERSION_MINOR "08")
set (RELEASE_SERVICE_VERSION_MICRO "3")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(KTorrent VERSION ${RELEASE_SERVICE_VERSION})


# We want RELEASE_SERVICE_VERSION_MINOR to have 08 but the integer define can't have a leading 0 since otherwise it's considered octal
# So strip a leading 0 if found in RELEASE_SERVICE_VERSION_MINOR and store the result in RELEASE_SERVICE_VERSION_MINOR_INT
string(REGEX REPLACE "^0?(.+)$" "\\1" RELEASE_SERVICE_VERSION_MINOR_INT "${RELEASE_SERVICE_VERSION_MINOR}")


add_definitions(-D'VERSION="${RELEASE_SERVICE_VERSION}"'
                -D'VERSION_MAJOR=${RELEASE_SERVICE_VERSION_MAJOR}'
                -D'VERSION_MINOR=${RELEASE_SERVICE_VERSION_MINOR_INT}'
                -D'VERSION_MICRO=${RELEASE_SERVICE_VERSION_MICRO}')

set (QT_MIN_VERSION "6.5.0")
set (KF_MIN_VERSION "5.240.0")
set (LIBKTORRENT_MIN_VERSION "24.04.70")
set (Boost_MIN_VERSION "1.71.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(KDEClangFormat)
include(KDEGitCommitHooks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED
    Core
    DBus
    Network
    Widgets
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED
  COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    GlobalAccel
    I18n
    IconThemes
    KIO
    Notifications
    NotifyConfig
    KCMUtils
    Parts
    Solid
    StatusNotifierItem
    WidgetsAddons
    WindowSystem
    XmlGui
  OPTIONAL_COMPONENTS
    DocTools
)

find_package(Qt6Core5Compat ${QT_MIN_VERSION})
set_package_properties(Qt6Core5Compat
  PROPERTIES
  TYPE REQUIRED
  PURPOSE "PUBLIC Required for libktorrent"
)

find_package(Qt6Test ${QT_MIN_VERSION} CONFIG QUIET)
set_package_properties(Qt6Test PROPERTIES
    PURPOSE "Required for tests"
    TYPE OPTIONAL
)
add_feature_info("Qt6Test" Qt6Test_FOUND "Required for building tests")
if (NOT Qt6Test_FOUND)
    set(BUILD_TESTING OFF CACHE BOOL "Build the testing tree.")
endif()

find_package(KTorrent6 ${LIBKTORRENT_MIN_VERSION} CONFIG REQUIRED)
find_package(Boost ${Boost_MIN_VERSION} REQUIRED)

find_package(KF6TextWidgets ${KF_MIN_VERSION})
set_package_properties(KF6TextWidgets
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's IP Filter plugin"
)
if(KF6TextWidgets_FOUND)
  set(HAVE_KF6TextWidgets 1)
endif()

find_package(KF6ItemViews ${KF_MIN_VERSION})
set_package_properties(KF6ItemViews
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's scripting plugin"
)
if(KF6ItemViews_FOUND)
  set(HAVE_KF6ItemViews 1)
endif()

find_package(KF6Plotting ${KF_MIN_VERSION})
set_package_properties(KF6Plotting
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's statistics plugin"
)
if(KF6Plotting_FOUND)
  set(HAVE_KF6Plotting 1)
endif()

find_package(KF6Syndication ${KF_MIN_VERSION})
set_package_properties(KF6Syndication
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's syndication plugin"
)
if(KF6Syndication_FOUND)
  set(HAVE_KF6Syndication 1)
endif()

find_package(Qt6WebEngineWidgets ${QT_MIN_VERSION})
set_package_properties(Qt6WebEngineWidgets
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's search and syndication plugins"
)
if(Qt6WebEngineWidgets_FOUND)
  set(HAVE_Qt6WebEngineWidgets 1)
endif()

find_package(KF6DNSSD ${KF_MIN_VERSION})
set_package_properties(KF6DNSSD
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's Zeroconf plugin"
)
if(KF6DNSSD_FOUND)
  set(HAVE_KF6DNSSD 1)
endif()

find_package(KF6Completion ${KF_MIN_VERSION})
set_package_properties(KF6Completion
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's UPnP plugin"
)
if(KF6Completion_FOUND)
  set(HAVE_KF6Completion 1)
endif()

#find_package(Qt6 ${QT_MIN_VERSION} OPTIONAL_COMPONENTS Multimedia MultimediaWidgets)

find_package(Taglib)
set_package_properties(Taglib
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's multimedia plugin"
)

if(TAGLIB_FOUND)
  set(HAVE_Taglib 1)
endif()

find_package(Phonon4Qt6)
set_package_properties(Phonon4Qt6
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's multimedia plugin"
)


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
  -DQT_NO_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f02)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055900)

set (KTORRENT_DBUS_XML_DIR ${CMAKE_SOURCE_DIR}/dbus_xml)
set (KTORRENT_PLUGIN_INSTALL_DIR ${PLUGIN_INSTALL_DIR}/ktorrent)


add_subdirectory(libktcore)
add_subdirectory(plugins)
add_subdirectory(ktorrent)
add_subdirectory(ktupnptest)
add_subdirectory(ktmagnetdownloader)
if (KF6DocTools_FOUND)
    add_subdirectory(doc)
    kdoctools_install(po)
endif()
ki18n_install(po)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)


feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
