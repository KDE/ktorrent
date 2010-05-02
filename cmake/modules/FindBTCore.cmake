# - Try to find the BTCore library
# Once done this will define
#
#  BTCORE_FOUND - system has BTCore
#  BTCORE_INCLUDE_DIR - the BTCore include directory
#  BTCORE_LIBRARIES - Link these to use BTCore

# Copyright (c) 2007 Joris Guisson <joris.guisson@gmail.com>
# Copyright (c) 2007 Charles Connell <charles@connells.org> (This was based upon FindKopete.cmake)
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(BTCORE_INCLUDE_DIR AND BTCORE_LIBRARIES)

  # read from cache
  set(BTCORE_FOUND TRUE)

else(BTCORE_INCLUDE_DIR AND BTCORE_LIBRARIES)

  FIND_PATH(BTCORE_INCLUDE_DIR 
    NAMES
    btcore_export.h
    PATHS 
    ${KDE4_INCLUDE_DIR}
    ${INCLUDE_INSTALL_DIR}
    PATH_SUFFIXES
    libbtcore
    )
  
  FIND_LIBRARY(BTCORE_LIBRARIES 
    NAMES
    btcore
    PATHS
    ${KDE4_LIB_DIR}
    ${LIB_INSTALL_DIR}
    )
  if(BTCORE_INCLUDE_DIR AND BTCORE_LIBRARIES)
    set(BTCORE_FOUND TRUE)
  endif(BTCORE_INCLUDE_DIR AND BTCORE_LIBRARIES)

  if(MSVC)
    FIND_LIBRARY(BTCORE_LIBRARIES_DEBUG 
      NAMES
      kopeted
      PATHS
      ${KDE4_LIB_DIR}
      ${LIB_INSTALL_DIR}
      )
    if(NOT BTCORE_LIBRARIES_DEBUG)
      set(BTCORE_FOUND FALSE)
    endif(NOT BTCORE_LIBRARIES_DEBUG)
    
    if(MSVC_IDE)
      if( NOT BTCORE_LIBRARIES_DEBUG OR NOT BTCORE_LIBRARIES)
        message(FATAL_ERROR "\nCould NOT find the debug AND release version of the BTCore library.\nYou need to have both to use MSVC projects.\nPlease build and install both BTCore libraries first.\n")
      endif( NOT BTCORE_LIBRARIES_DEBUG OR NOT BTCORE_LIBRARIES)
    else(MSVC_IDE)
      string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
      if(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
        set(BTCORE_LIBRARIES ${BTCORE_LIBRARIES_DEBUG})
      else(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
        set(BTCORE_LIBRARIES ${BTCORE_LIBRARIES})
      endif(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
    endif(MSVC_IDE)
  endif(MSVC)

  if(BTCORE_FOUND)
    if(NOT BTCORE_FIND_QUIETLY)
      message(STATUS "Found BTCore: ${BTCORE_LIBRARIES} ")
    endif(NOT BTCORE_FIND_QUIETLY)
  else(BTCORE_FOUND)
    if(BTCORE_FIND_REQUIRED)
      if(NOT BTCORE_INCLUDE_DIR)
	message(FATAL_ERROR "Could not find BTCore includes.")
      endif(NOT BTCORE_INCLUDE_DIR)
      if(NOT BTCORE_LIBRARIES)
	message(FATAL_ERROR "Could not find BTCore library.")
      endif(NOT BTCORE_LIBRARIES)
    else(BTCORE_FIND_REQUIRED)
      if(NOT BTCORE_INCLUDE_DIR)
        message(STATUS "Could not find BTCore includes.")
      endif(NOT BTCORE_INCLUDE_DIR)
      if(NOT BTCORE_LIBRARIES)
        message(STATUS "Could not find BTCore library.")
      endif(NOT BTCORE_LIBRARIES)
    endif(BTCORE_FIND_REQUIRED)
  endif(BTCORE_FOUND)

endif(BTCORE_INCLUDE_DIR AND BTCORE_LIBRARIES)
