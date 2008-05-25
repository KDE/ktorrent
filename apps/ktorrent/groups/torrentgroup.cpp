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
#include <util/log.h>
#include <util/error.h>
#include <util/sha1hash.h>
#include <torrent/bencoder.h>
#include <torrent/bnode.h>
#include <interfaces/torrentinterface.h>
#include "torrentgroup.h"

using namespace bt;

namespace kt
{

	TorrentGroup::TorrentGroup(const QString& name): Group(name,MIXED_GROUP|CUSTOM_GROUP)
	{
		setIconByName("player_playlist");
	}


	TorrentGroup::~TorrentGroup()
	{}


	bool TorrentGroup::isMember(TorrentInterface* tor)
	{
		if (torrents.count(tor) > 0)
			return true;
		
		
		if (!hashes.empty())
		{
			if (hashes.count(tor->getInfoHash()))
			{
		/*		bt::Out(SYS_GEN|LOG_DEBUG) << 
						QString("TG %1 : Torrent %2 from hashes list").arg(groupName()).arg(tor->getStats().torrent_name) << endl;
		*/ 
				hashes.erase(tor->getInfoHash());
				torrents.insert(tor);
				return true;
			}
		}
		return false;
	}

	void TorrentGroup::add(TorrentInterface* tor)
	{
		torrents.insert(tor);
	}
	
	void TorrentGroup::remove(TorrentInterface* tor)
	{
		torrents.erase(tor);
	}
	
	void TorrentGroup::save(bt::BEncoder* enc)
	{
		enc->beginDict();
		enc->write("name"); enc->write(name.local8Bit());
		enc->write("icon"); enc->write(icon_name.local8Bit());
		enc->write("hashes"); enc->beginList();
		std::set<TorrentInterface*>::iterator i = torrents.begin();
		while (i != torrents.end())
		{
			TorrentInterface* tc = *i;
			// write the info hash, because that will be unique for each torrent
			const bt::SHA1Hash & h = tc->getInfoHash();
			enc->write(h.getData(),20);
			i++;
		}
		std::set<bt::SHA1Hash>::iterator j = hashes.begin();
		while (j != hashes.end())
		{
			enc->write(j->getData(),20);
			j++;
		}
		enc->end();
		enc->end();
	}
	
	void TorrentGroup::load(bt::BDictNode* dn)
	{
		BValueNode* vn = dn->getValue("name");
		if (!vn || vn->data().getType() != bt::Value::STRING)
			throw bt::Error("invalid or missing name");
		
		QByteArray tmp = vn->data().toByteArray();
		name = QString::fromLocal8Bit(tmp.data(),tmp.size());
		
		vn = dn->getValue("icon");
		if (!vn || vn->data().getType() != bt::Value::STRING)
			throw bt::Error("invalid or missing icon");
		
		tmp = vn->data().toByteArray();
		setIconByName(QString::fromLocal8Bit(tmp.data(),tmp.size()));
		
		BListNode* ln = dn->getList("hashes");
		if (!ln)
			return;
		
		for (Uint32 i = 0;i < ln->getNumChildren();i++)
		{
			vn = ln->getValue(i);
			if (!vn || vn->data().getType() != bt::Value::STRING)
				continue;
			
			QByteArray ba = vn->data().toByteArray();
			if (ba.size() != 20)
				continue;
			
			hashes.insert(SHA1Hash((const Uint8*)ba.data()));
		}
	}
	
	void TorrentGroup::torrentRemoved(TorrentInterface* tor)
	{
		torrents.erase(tor);
	}
	
	void TorrentGroup::removeTorrent(TorrentInterface* tor)
	{
		torrents.erase(tor);
	}
	
	void TorrentGroup::addTorrent(TorrentInterface* tor)
	{
		torrents.insert(tor);
	}
}
