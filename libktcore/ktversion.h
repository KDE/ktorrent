/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KTVERSION_HH
#define KTVERSION_HH

#include "util/constants.h"
#include <version.h>

namespace kt
{
const bt::Uint32 MAJOR = VERSION_MAJOR;
const bt::Uint32 MINOR = VERSION_MINOR;
const bt::Uint32 RELEASE = VERSION_MICRO;
const bt::VersionType VERSION_TYPE = bt::NORMAL;
}

#endif
