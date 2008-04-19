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
#include <time.h>
#include <stdlib.h>
#include <qmap.h>
#include <klocale.h>
#include "peerid.h"
#include "ktversion.h"

namespace bt
{
	char RandomLetterOrNumber()
	{
		int i = rand() % 62;
		if (i < 26)
			return 'a' + i;
		else if (i < 52)
			return 'A' + (i - 26);
		else
			return '0' + (i - 52);
	}

	
	PeerID::PeerID()
	{
		srand(time(0));
		memcpy(id,kt::PEER_ID,8);
		for (int i = 8;i < 20;i++)
			id[i] = RandomLetterOrNumber(); 
		client_name = identifyClient();
	}

	PeerID::PeerID(const char* pid)
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
			// Keep things a bit alphabetic to make it easier add new ones
			//AZUREUS STYLE
			Map["AG"] = "Ares";
			Map["A~"] = "Ares";
			Map["AV"] = "Avicora";
			Map["AX"] = "BitPump";
			Map["AR"] = "Arctic";
			Map["AZ"] = "Azureus";
			Map["BB"] = "BitBuddy";
			Map["BC"] = "BitComet";
			Map["BF"] = "Bitflu";
			Map["BG"] = "BTGetit";
			Map["BM"] = "BitMagnet";
			Map["BO"] = "BitsOnWheels";
			Map["BR"] = "BitRocket";
			Map["BS"] = "BTSlave"; 
			Map["BX"] = "BitTorrent X";
			Map["CD"] = "Enhanced CTorrent";
			Map["CT"] = "CTorrent";
			Map["DE"] = "DelugeTorrent";
			Map["DP"] = "Propagate Data Client";
			Map["EB"] = "EBit";
			Map["ES"] = "electric sheep";
			Map["FT"] = "FoxTorrent";
			Map["GS"] = "GSTorrent";
			Map["G3"] = "G3 Torrent";
			Map["HL"] = "Halite";
			Map["HN"] = "Hydranode";
			Map["KG"] = "KGet";
			Map["KT"] = "KTorrent"; // lets not forget our own client
			Map["LH"] = "LH-ABC";
			Map["lt"] = "libTorrent";
			Map["LT"] = "libtorrent";
			Map["LP"] = "Lphant";
			Map["LW"] = "LimeWire";
			Map["ML"] = "MLDonkey";
			Map["MO"] = "MonoTorrent";
			Map["MP"] = "MooPolice";
			Map["MT"] = "MoonLight";
			Map["PD"] = "Pando";
			Map["qB"] = "qBittorrent";
			Map["QD"] = "QQDownload";
			Map["QT"] = "Qt 4 Torrent example";
			Map["RS"] = "Rufus";
			Map["RT"] = "Retriever";
			Map["S~"] = "Shareaza alpha/beta";
			Map["SB"] = "Swiftbit";
			Map["SS"] = "SwarmScope";
			Map["ST"] = "SymTorrent";
			Map["st"] = "sharktorrent";
			Map["SZ"] = "Shareaza";
			Map["TN"] = "Torrent .NET";
			Map["TR"] = "Transmission";
			Map["TS"] = "Torrent Storm";
			Map["TT"] = "TuoTu";
			Map["UL"] = "uLeecher!";
			Map["UT"] = QString("%1Torrent").arg(QChar(0x00B5)); // µTorrent, 0x00B5 is unicode for µ
			Map["WT"] = "BitLet";
			Map["WY"] = "FireTorrent";
			Map["XL"] = "Xunlei";
			Map["XT"] = "Xan Torrent";
			Map["XX"] = "Xtorrent";
			Map["ZT"] = "Zip Torrent";
			
			//SHADOWS STYLE
			Map["A"] = "ABC";
			Map["O"] = "Osprey Permaseed";
			Map["Q"] = "BTQueue";
			Map["R"] = "Tribler";
			Map["S"] = "Shadow's";
			Map["T"] = "BitTornado";
			Map["U"] = "UPnP NAT BitTorrent";
			//OTHER
			Map["Plus"] = "Plus! II";
			Map["OP"] = "Opera";
			Map["BOW"] = "Bits on Wheels";
			Map["M"] = "BitTorrent";
			Map["exbc"] = "BitComet";
			Map["Mbrst"] = "Burst!";
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
		else if (peer_id.at(0) == 'M' && peer_id.at(2) == '-' && (peer_id.at(4) == '-' || peer_id.at(5) == '-'))
		{
			name = Map["M"] + " " + peer_id.at(1) + "." + peer_id.at(3);
			if(peer_id.at(4) == '-')
				name += "." + peer_id.at(5);
			else
				name += peer_id.at(4) + "." + peer_id.at(6);
		}
		else if (peer_id.startsWith("OP"))
		{
			name = Map["OP"];
		}
		else if ( peer_id.startsWith("exbc") )
		{
			name = Map["exbc"];
		}
		else if ( peer_id.mid(1,3) == "BOW")
		{
			name = Map["BOW"];
		}
		else if ( peer_id.startsWith("Plus"))
		{
			name = Map["Plus"];
		}
		else if ( peer_id.startsWith("Mbrst"))
		{
			name = Map["Mbrst"] + " " + peer_id.at(5) + "." + peer_id.at(7);
		}
			
		return name;
	}
}
