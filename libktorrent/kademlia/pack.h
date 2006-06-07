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
#ifndef DHTPACK_H
#define DHTPACK_H

#include "kbucket.h"

namespace dht
{

	/**
	 * Pack a KBucketEntry into a byte array.
	 * If the array is not large enough, an error will be thrown
	 * @param e The entry
	 * @param ba The byte array
	 * @param off The offset into the array
	 */
	void PackBucketEntry(const KBucketEntry & e,QByteArray & ba,Uint32 off);
	
	/**
	 * Unpack a KBucketEntry from a byte array.
	 * If a full entry cannot be read an error will be thrown.
	 * @param ba The byte array
	 * @param off The offset
	 * @return The entry
	 */
	KBucketEntry UnpackBucketEntry(const QByteArray & ba,Uint32 off);

}

#endif
