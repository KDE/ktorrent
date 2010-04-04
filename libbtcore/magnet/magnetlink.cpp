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

#include "magnetlink.h"
#include <KUrl>
#include <util/log.h>
#include <util/error.h>

namespace bt
{
	
	MagnetLink::MagnetLink()
	{
	}

	MagnetLink::MagnetLink(const MagnetLink& mlink)
	{
		magnet_string = mlink.magnet_string;
		info_hash = mlink.info_hash;
		tracker_url = mlink.tracker_url;
		name = mlink.name;
	}

	MagnetLink::MagnetLink(const QString& mlink)
	{
		parse(mlink);
	}
	
	MagnetLink::~MagnetLink()
	{
	}

	MagnetLink& MagnetLink::operator=(const bt::MagnetLink& mlink)
	{
		magnet_string = mlink.magnet_string;
		info_hash = mlink.info_hash;
		tracker_url = mlink.tracker_url;
		name = mlink.name;
		return *this;
	}

	bool MagnetLink::operator==(const bt::MagnetLink& mlink)
	{
		return info_hash == mlink.infoHash();
	}
	
	void MagnetLink::parse(const QString& mlink)
	{
		KUrl url(mlink);
		if (url.protocol() != "magnet")
		{
			Out(SYS_GEN|LOG_NOTICE) << "Invalid magnet link " << mlink << endl;
			return;
		}
		
		QString xt = url.queryItem("xt");
		if (!xt.startsWith("urn:btih:"))
		{
			Out(SYS_GEN|LOG_NOTICE) << "Invalid magnet link " << mlink << endl;
			return;
		}
		
		QString ih = xt.mid(9);
		if (ih.length() != 40 && ih.length() != 32)
		{
			Out(SYS_GEN|LOG_NOTICE) << "Invalid magnet link " << mlink << endl;
			return;
		}
		
		try
		{
			if (ih.length() == 32)
				ih = base32ToHexString(ih);

			Uint8 hash[20];
			memset(hash,0,20);
			for (int i = 0;i < 20;i++)
			{
				Uint8 low = charToHex(ih[2*i + 1]);
				Uint8 high = charToHex(ih[2*i]);
				hash[i] = (high << 4) | low;
			}
			
			info_hash = SHA1Hash(hash);
			tracker_url = url.queryItem("tr");
			name = url.queryItem("dn");
			magnet_string = mlink;
		}
		catch (...)
		{
			Out(SYS_GEN|LOG_NOTICE) << "Invalid magnet link " << mlink << endl;
		}
	}
	
	Uint8 MagnetLink::charToHex(const QChar& ch)
	{
		if (ch.isDigit())
			return ch.digitValue();
		
		if (!ch.isLetter())
			throw bt::Error("Invalid char");
		
		if (ch.isLower())
			return 10 + ch.toAscii() - 'a';
		else
			return 10 + ch.toAscii() - 'A';
	}

	QString MagnetLink::base32ToHexString(const QString &s)
	{
		Uint32 part;
		Uint32 tmp;
		QString ret("");
		QChar ch;
		QString str = s.toUpper();
		// 32 base32 chars -> 40 hex chars
		// 4 base32 chars -> 5 hex chars
		for (int i = 0; i < 8; i++)
		{
			part = 0;
			for (int j = 0; j < 4; j++)
			{
				ch = str[i*4 + j];
				if (ch.isDigit() && (ch.digitValue() < 2 || ch.digitValue() > 7))
					throw bt::Error("Invalid char");

				if (ch.isDigit())
					 tmp = ch.digitValue() + 24;
				else
					tmp = ch.toAscii() - 'A';
				part = part + (tmp << 5*(3-j));
			}

			// part is a Uint32 with 20 bits (5 hex)
			for (int j = 0; j < 5; j++)
			{
				tmp = (part >> 4*(4-j)) & 0xf;
				if (tmp >= 10)
					ret.append(QChar((tmp-10) + 'a'));
				else
					ret.append(QChar(tmp + '0'));
			}
		}
		return ret;
	}

}

