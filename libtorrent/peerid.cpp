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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <time.h>
#include <stdlib.h>
#include <qmap.h>
#include <klocale.h>
#include "peerid.h"

namespace bt
{
	
	PeerID::PeerID()
	{
		srand(time(0));
		int r[12];
		for (int i = 0;i < 12;i++)
			r[i] = rand() % 10;
		QString peer_id = "-KT11R1-";
		for (int i = 0;i < 12;i++)
			peer_id += QString("%1").arg(r[i]);
		memcpy(id,peer_id.ascii(),20);

		client_name = identifyClient();
	}

	PeerID::PeerID(char* pid)
	{
		if (pid)
			memcpy(id,pid,20);
		else
			memset(id,0,20);
		client_name = identifyClient();
	}

	PeerID::PeerID(const PeerID & pid)
	{
		memcpy(id,pid.id,20);
		client_name = pid.client_name;
	}

	PeerID::~PeerID()
	{}



	PeerID & PeerID::operator = (const PeerID & pid)
	{
		memcpy(id,pid.id,20);
		client_name = pid.client_name;
		return *this;
	}

	bool operator == (const PeerID & a,const PeerID & b)
	{
		for (int i = 0;i < 20;i++)
			if (a.id[i] != b.id[i])
				return false;

		return true;
	}

	bool operator != (const PeerID & a,const PeerID & b)
	{
		return ! operator == (a,b);
	}

	bool operator < (const PeerID & a,const PeerID & b)
	{
		for (int i = 0;i < 20;i++)
			if (a.id[i] < b.id[i])
				return true;

		return false;
	}

	QString PeerID::toString() const
	{
		QString r;
		for (int i = 0;i < 20;i++)
			r += id[i] == 0 ? ' ' : id[i];
		return r;
	}

	QString PeerID::identifyClient() const
	{
		if (!client_name.isNull())
			return client_name;
		
		QString peer_id = toString();
		// we only need to create this map once
		// so make it static
		static QMap<QString, QString> Map;
		static bool first = true; 

		if (first)
		{
			//AZUREUS STYLE
			Map["AZ"] = "Azureus";
			Map["BC"] = "BitComet";
			Map["BB"] = "BitBuddy";
			Map["BX"] = "BitTorrent X";
			Map["CT"] = "CTorrent";
			Map["G3"] = "G3 Torrent";
			Map["MT"] = "MoonLight";
			Map["LT"] = "LibTorrent";
			Map["SS"] = "SwarmScope";
			Map["TN"] = "Torrent .NET";
			Map["TS"] = "Torrent Storm";
			Map["XT"] = "Xan Torrent";
			Map["ZT"] = "Zip Torrent";
			Map["KT"] = "KTorrent"; // lets not forget our own client
			//SHADOWS STYLE
			Map["A"] = "ABC";
			Map["S"] = "Shadow's";
			Map["T"] = "BitTornado";
			Map["U"] = "UPnP NAT BitTorrent";
			//OTHER
			Map["M"] = "BitTorrent";
			Map["exbc"] = "BitComet";
			Map["Mbrst"] = "burst!";
			first = false;
		}

		QString name = i18n("Unknown client");
		if (peer_id.at(0) == '-' &&
			peer_id.at(1).isLetter() &&
			peer_id.at(2).isLetter() ) //AZ style
		{
			QString ID(peer_id.mid(1,2));
			if (Map.contains(ID))
				name = Map[ID] + " " + peer_id.at(3) + "." + peer_id.at(4) + "."
					+ peer_id.at(5) + "." + peer_id.at(6);
		}
		else if (peer_id.at(0).isLetter() &&
				peer_id.at(1).isDigit() &&
				peer_id.at(2).isDigit() )  //Shadow's style
		{
			QString ID = QString(peer_id.at(0));
			if (Map.contains(ID))
				name = Map[ID] + " " + peer_id.at(1) + "." +
						peer_id.at(2) + "." + peer_id.at(3);
		}
		else if (peer_id.at(0) == 'M' && peer_id.at(2) == '-' && peer_id.at(4) == '-' )
		{
			name = Map["M"] + " " + peer_id.at(1) + "." +
					peer_id.at(3) + "." + peer_id.at(5);
		}
		
		if ( peer_id.startsWith("exbc") )
			name = Map["exbc"];
		return name;
	}
}
