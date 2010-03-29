/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef BTVERSION_H
#define BTVERSION_H

#include <btcore_export.h>
#include <util/constants.h>

class QString;

namespace bt 
{
	enum VersionType
	{
		NORMAL,ALPHA,BETA,RELEASE_CANDIDATE,DEVEL
	};
	
	/**
	 * Set the client info. This information is used to create
	 * the PeerID and the version string (used in HTTP announces for example).
	 * @param name Name of the client
	 * @param major Major version
	 * @param minor Minor version
	 * @param release Release version
	 * @param type Which version 
	 * @param peer_id_code Peer ID code (2 letters identifying the client, KT for KTorrent)
	 */
	BTCORE_EXPORT void SetClientInfo(const QString & name,int major,int minor,int release,VersionType type,const QString & peer_id_code);

	/**
	 * Get the PeerID prefix set by SetClientInfo
	 * @return The PeerID prefix
	 */
	BTCORE_EXPORT QString PeerIDPrefix();
	
	/**
	 * Get the current client version string
	 */
	BTCORE_EXPORT QString GetVersionString();
	
	
	/// Major version number of the BTCore library
	const Uint32 MAJOR = 4;
	/// Minor version number of the BTCore library
	const Uint32 MINOR = 0;
	/// Version type of the BTCore library
	const VersionType VERSION_TYPE = BETA;
	/// Release version number of the BTCore library
	const Uint32 RELEASE = 2;
}

#endif
