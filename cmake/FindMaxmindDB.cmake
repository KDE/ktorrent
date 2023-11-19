# SPDX-FileCopyrightText: 2006 Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2019 Heiko Becker <heirecka@exherbo.org>
# SPDX-FileCopyrightText: 2020 Elvis Angelaccio <elvis.angelaccio@kde.org>
# SPDX-FileCopyrightText: 2023 Gustavo Alvarez <sl1pkn07@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
FindMaxmindDB
----------

Try to find the MaxmindDB library.

This will define the following variables:

``MaxmindDB_FOUND``
      True if the system has the libmaxminddb library of at least the minimum
      version specified by the version parameter to find_package()
``MaxmindDB_VERSION``
      The version of libmaxminddb
``MaxmindDB_INCLUDE_DIRS``
      The libmaxminddb include dirs for use with target_include_directories
``MaxmindDB_LIBRARIES``
      The libmaxminddb libraries for use with target_link_libraries()

If ``MaxmindDB_FOUND`` is TRUE, it will also define the following imported
target:

``MaxmindDB::MaxmindDB``
      The MaxmindDB library

#]=======================================================================]

find_package(PkgConfig QUIET)

pkg_check_modules(PC_MAXMINDDB QUIET libmaxminddb)
set(MaxmindDB_VERSION ${PC_MAXMINDDB_VERSION})

find_path(MaxmindDB_INCLUDE_DIRS
    NAMES maxminddb.h
    HINTS ${PC_MAXMINDDB_INCLUDEDIR}
)

find_library(MaxmindDB_LIBRARIES
    NAMES maxminddb
    HINTS ${PC_MAXMINDDB_LIBDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MaxmindDB
    FOUND_VAR
        MaxmindDB_FOUND
    REQUIRED_VARS
        MaxmindDB_LIBRARIES
        MaxmindDB_INCLUDE_DIRS
    VERSION_VAR
        MaxmindDB_VERSION
)

if (MaxmindDB_FOUND AND NOT TARGET MaxmindDB::MaxmindDB)
    add_library(MaxmindDB::MaxmindDB UNKNOWN IMPORTED)
    set_target_properties(MaxmindDB::MaxmindDB PROPERTIES
        IMPORTED_LOCATION "${MaxmindDB_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${MaxmindDB_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(MaxmindDB_LIBRARIES MaxmindDB_INCLUDE_DIRS MaxmindDB_VERSION)

include(FeatureSummary)
set_package_properties(MaxmindDB PROPERTIES
    URL "https://www.maxmind.com/"
    DESCRIPTION "A library for Non-DNS IP-to-country resolver"
)
