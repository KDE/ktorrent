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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "peersource.h"

namespace kt
{

	PeerSource::PeerSource() 
	{}


	PeerSource::~PeerSource()
	{}
	
	void PeerSource::completed()
	{}
	
	void PeerSource::manualUpdate()
	{}
	
	void PeerSource::aboutToBeDestroyed()
	{}
	
	void PeerSource::addPeer(const QString & ip,bt::Uint16 port,bool local)
	{
		PotentialPeer pp;
		pp.ip = ip;
		pp.port = port;
		pp.local = local;
		peers.append(pp);
	}
		
	bool PeerSource::takePotentialPeer(PotentialPeer & pp)
	{
		if (peers.count() > 0)
		{
			pp = peers.front();
			peers.pop_front();
			return true;
		}
		return false;
	}

	

}
#include "peersource.moc"
