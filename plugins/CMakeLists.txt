MACRO (MACRO_KT_PLUGIN _option _name _dir)
option(${_option} "Whether to build the ${_name} plugin or not" true)
if (${_option})
  message(STATUS "Building ${_name} plugin (-D${_option}=false to disable)")
  add_subdirectory(${_dir})
else (${_option})
  message(STATUS "Not building ${_name} plugin (-D${_option}=true to enable)")
endif (${_option})
ENDMACRO (MACRO_KT_PLUGIN)

find_package(Taglib)
#find_package(Qt5 ${QT_MIN_VERSION} OPTIONAL_COMPONENTS Multimedia MultimediaWidgets)
find_package(Phonon4Qt5)

find_package(KF5 5.15 OPTIONAL_COMPONENTS WebKit IconThemes DNSSD Plotting)

find_package(LibKWorkspace CONFIG)
set_package_properties(LibKWorkspace
  PROPERTIES
  TYPE OPTIONAL
  PURPOSE "Required for KTorrent's shutdown plugin"
)

macro_kt_plugin(ENABLE_LOGVIEWER_PLUGIN logviewer logviewer)
macro_kt_plugin(ENABLE_INFOWIDGET_PLUGIN infowidget infowidget)
macro_kt_plugin(ENABLE_UPNP_PLUGIN upnp upnp)
if (KF5_WebKit_FOUND AND KF5_IconThemes_FOUND)
#  macro_kt_plugin(ENABLE_SEARCH_PLUGIN search search)
endif()
#macro_kt_plugin(ENABLE_WEBINTERFACE_PLUGIN webinterface webinterface)
#macro_kt_plugin(ENABLE_SCANFOLDER_PLUGIN scanfolder scanfolder)
#macro_kt_plugin(ENABLE_IPFILTER_PLUGIN ipfilter ipfilter)
macro_kt_plugin(ENABLE_STATS_PLUGIN stats stats)
macro_kt_plugin(ENABLE_BWSCHEDULER_PLUGIN bwscheduler bwscheduler)
if (TAGLIB_FOUND AND PHONON_FOUND_EXPERIMENTAL) #AND Qt5Multimedia_FOUND)
  macro_kt_plugin(ENABLE_MEDIAPLAYER_PLUGIN mediaplayer mediaplayer)
endif()
#macro_kt_plugin(ENABLE_SCRIPTING_PLUGIN scripting scripting)
#macro_kt_plugin(ENABLE_SYNDICATION_PLUGIN syndication syndication)
macro_kt_plugin(ENABLE_DOWNLOADORDER_PLUGIN downloadorder downloadorder)
if (LibKWorkspace_FOUND)
  macro_kt_plugin(ENABLE_SHUTDOWN_PLUGIN shutdown shutdown)
endif()
if (KF5_DNSSD_FOUND)
  macro_kt_plugin(ENABLE_ZEROCONF_PLUGIN zeroconf zeroconf)
endif()
macro_kt_plugin(ENABLE_MAGNETGENERATOR_PLUGIN magnetgenerator magnetgenerator)

#macro_kt_plugin(ENABLE_STREAMING_PLUGIN streaming streaming)
