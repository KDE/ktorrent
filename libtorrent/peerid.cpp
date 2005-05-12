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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <time.h>
#include <stdlib.h>
#include "peerid.h"

namespace bt
{

	PeerID::PeerID()
	{
		srand(time(0));
		int r[12];
		for (int i = 0;i < 12;i++)
			r[i] = rand() % 10;
		QString peer_id = "-KT0900-";
		for (int i = 0;i < 12;i++)
			peer_id += QString("%1").arg(r[i]);
		memcpy(id,peer_id.ascii(),20);
	}

	PeerID::PeerID(char* pid)
	{
		memcpy(id,pid,20);
	}
	
	PeerID::PeerID(const PeerID & pid)
	{
		memcpy(id,pid.id,20);
	}

	PeerID::~PeerID()
	{}
	


	PeerID & PeerID::operator = (const PeerID & pid)
	{
		memcpy(id,pid.id,20);
		return *this;
	}
		
	bool operator == (const PeerID & a,const PeerID & b)
	{
		for (int i = 0;i < 20;i++)
			if (a.id[i] != b.id[i])
				return false;
		
		return true;
	}

	QString PeerID::toString() const
	{
		QString r;
		for (int i = 0;i < 20;i++)
			r += id[i] == 0 ? ' ' : id[i];
		return r;
	}
}
