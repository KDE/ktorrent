/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTVERSION_HH
#define KTVERSION_HH

#include <btversion.h>
#include "util/constants.h"

namespace kt
{
	const bt::Uint32 MAJOR = 3;
	const bt::Uint32 MINOR = 2;
	const bt::Uint32 RELEASE = 3;
	const bt::VersionType VERSION_TYPE = bt::NORMAL;
	const char VERSION_STRING[] = "3.2.3";
}

#define KT_VERSION_MACRO "3.2.3"

#endif
