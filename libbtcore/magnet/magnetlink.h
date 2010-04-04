/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/


#ifndef BT_MAGNETLINK_H
#define BT_MAGNETLINK_H

#include <QString>
#include <btcore_export.h>
#include <util/sha1hash.h>

namespace bt
{
	/**
		MagnetLink class
		magnet links have the format: 
		magnet:?xt=urn:btih:info_hash&dn=name&tr=tracker-url
	*/
	class BTCORE_EXPORT MagnetLink
	{
	public:
		MagnetLink();
		MagnetLink(const MagnetLink & mlink);
		MagnetLink(const QString & mlink);
		virtual ~MagnetLink();
		
		/// Assignment operator
		MagnetLink & operator = (const MagnetLink & mlink);
		
		/// Equality operator
		bool operator == (const MagnetLink & mlink);
		
		/// Is this a valid magnet link
		bool isValid() const {return !magnet_string.isEmpty();}
		
		/// Convert it to a string
		QString toString() const {return magnet_string;}
		
		/// Get the display name (can be empty)
		QString displayName() const {return name;}
		
		/// Get the tracker URL (can be empty)
		QString tracker() const {return tracker_url;}
		
		/// Get the info hash
		const SHA1Hash & infoHash() const {return info_hash;}
		
	private:
		void parse(const QString & mlink);
		Uint8 charToHex(const QChar & ch);
		QString base32ToHexString(const QString &s);

	private:
		QString magnet_string;
		SHA1Hash info_hash;
		QString tracker_url;
		QString name;
	};

}

#endif // BT_MAGNETLINK_H
