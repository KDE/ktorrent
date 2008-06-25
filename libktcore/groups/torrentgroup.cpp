/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
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
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include <interfaces/torrentinterface.h>
#include "torrentgroup.h"

using namespace bt;

namespace kt
{

	TorrentGroup::TorrentGroup(const QString& name): Group(name,MIXED_GROUP|CUSTOM_GROUP,"/all/custom/" + name)
	{
		setIconByName("application-x-bittorrent");
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
		enc->write(QString("name")); enc->write(name.toLocal8Bit());
		enc->write(QString("icon")); enc->write(icon_name.toLocal8Bit());
		enc->write(QString("hashes")); enc->beginList();
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
		enc->write(QString("policy")); enc->beginDict();
		enc->write(QString("default_save_location")); enc->write(policy.default_save_location);
		enc->write(QString("max_share_ratio")); enc->write(QString::number(policy.max_share_ratio));
		enc->write(QString("max_seed_time")); enc->write(QString::number(policy.max_seed_time));
		enc->write(QString("max_upload_rate")); enc->write(policy.max_upload_rate);
		enc->write(QString("max_download_rate")); enc->write(policy.max_download_rate);
		enc->write(QString("only_apply_on_new_torrents")); 
		enc->write((bt::Uint32) (policy.only_apply_on_new_torrents ? 1 : 0));
		enc->end();
		enc->end();
	}
	
	void TorrentGroup::load(bt::BDictNode* dn)
	{
		BValueNode* vn = dn->getValue("name");
		if (!vn || vn->data().getType() != bt::Value::STRING)
			throw bt::Error("invalid or missing name");
		
		name = QString::fromLocal8Bit(vn->data().toByteArray());
		
		vn = dn->getValue("icon");
		if (!vn || vn->data().getType() != bt::Value::STRING)
			throw bt::Error("invalid or missing icon");
		
		//setIconByName(QString::fromLocal8Bit(vn->data().toByteArray()));
		
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
		
		BDictNode* gp = dn->getDict(QString("policy"));
		if (gp)
		{
			// load the group policy
			vn = gp->getValue("default_save_location");
			if (vn && vn->data().getType() == bt::Value::STRING)
			{
				policy.default_save_location = vn->data().toString();
				if (policy.default_save_location.length() == 0)
					policy.default_save_location = QString(); // make sure that 0 length strings are loaded as null strings
			}
			
			vn = gp->getValue("max_share_ratio");
			if (vn && vn->data().getType() == bt::Value::STRING)
				policy.max_share_ratio = vn->data().toString().toFloat();
			
			vn = gp->getValue("max_seed_time");
			if (vn && vn->data().getType() == bt::Value::STRING)
				policy.max_seed_time = vn->data().toString().toFloat();
			
			vn = gp->getValue("max_upload_rate");
			if (vn && vn->data().getType() == bt::Value::INT)
				policy.max_upload_rate = vn->data().toInt();
			
			vn = gp->getValue("max_download_rate");
			if (vn && vn->data().getType() == bt::Value::INT)
				policy.max_download_rate = vn->data().toInt();
			
			vn = gp->getValue("only_apply_on_new_torrents");
			if (vn && vn->data().getType() == bt::Value::INT)
				policy.only_apply_on_new_torrents = vn->data().toInt();
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
	
	void TorrentGroup::addTorrent(TorrentInterface* tor,bool new_torrent)
	{
		torrents.insert(tor);
		// apply group policy if needed
		if (policy.only_apply_on_new_torrents && !new_torrent)
			return;
		
		tor->setMaxShareRatio(policy.max_share_ratio);
		tor->setMaxSeedTime(policy.max_seed_time);
		tor->setTrafficLimits(policy.max_upload_rate * 1024,policy.max_download_rate * 1024);
	}
	
	void TorrentGroup::policyChanged()
	{
		if (policy.only_apply_on_new_torrents)
			return;
		
		std::set<TorrentInterface*>::iterator i = torrents.begin();
		while (i != torrents.end())
		{
			TorrentInterface* tor = *i;
			tor->setMaxShareRatio(policy.max_share_ratio);
			tor->setMaxSeedTime(policy.max_seed_time);
			tor->setTrafficLimits(policy.max_upload_rate * 1024,policy.max_download_rate * 1024);
			i++;
		}
	}
}
