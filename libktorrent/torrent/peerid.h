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
#ifndef BTPEERID_H
#define BTPEERID_H

#include <qstring.h>

namespace bt
{

	/**
	@author Joris Guisson
	*/
	class PeerID
	{
		char id[20];
		QString client_name;
	public:
		PeerID();
		PeerID(const char* pid);
		PeerID(const PeerID & pid);
		virtual ~PeerID();

		PeerID & operator = (const PeerID & pid);
		
		const char* data() const {return id;}
		
		QString toString() const;

		/**
		 * Interprets the PeerID to figure out which client it is.
		 * @author Ivan + Joris
		 * @return The name of the client
		 */
		QString identifyClient() const;
		
		friend bool operator == (const PeerID & a,const PeerID & b);
		friend bool operator != (const PeerID & a,const PeerID & b);
		friend bool operator < (const PeerID & a,const PeerID & b);
	};

}

#endif
