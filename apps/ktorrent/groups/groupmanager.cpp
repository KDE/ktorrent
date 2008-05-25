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
#include <util/file.h>
#include <util/fileops.h>
#include <util/error.h>
#include <torrent/bnode.h>
#include <torrent/bdecoder.h>
#include <torrent/bencoder.h>
#include "groupmanager.h"
#include "torrentgroup.h"
#include "allgroup.h"
#include "downloadgroup.h"
#include "uploadgroup.h"
#include "torrentgroup.h"
#include "queueddownloadsgroup.h"
#include "queueduploadsgroup.h"
#include "userdownloadsgroup.h"
#include "useruploadsgroup.h"
#include "inactivegroup.h"
#include "inactivedownloadsgroup.h"
#include "inactiveuploadsgroup.h"
#include "activegroup.h"
#include "activedownloadsgroup.h"
#include "activeuploadsgroup.h"

using namespace bt;

namespace kt
{

	GroupManager::GroupManager()
	{
		setAutoDelete(true);
		default_groups.setAutoDelete(true);
		
		Group* g = new AllGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new UploadGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new DownloadGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new QueuedDownloadsGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new QueuedUploadsGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new UserDownloadsGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new UserUploadsGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new ActiveGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new ActiveUploadsGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new ActiveDownloadsGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new InactiveGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new InactiveUploadsGroup();
		default_groups.insert(g->groupName(),g);
		
		g = new InactiveDownloadsGroup();
		default_groups.insert(g->groupName(),g);
	}


	GroupManager::~GroupManager()
	{
	}


	Group* GroupManager::newGroup(const QString & name)
	{
		if (find(name))
			return 0;
		
		Group* g = new TorrentGroup(name);
		insert(name,g);
		return g;
	}
	
	bool GroupManager::canRemove(const Group* g) const
	{
		return default_groups.find(g->groupName()) == 0;
	}
	
	
	void GroupManager::saveGroups(const QString & fn)
	{
		bt::File fptr;
		if (!fptr.open(fn,"wb"))
		{
			bt::Out() << "Cannot open " << fn << " : " << fptr.errorString() << bt::endl;
			return;
		}
		
		try
		{	
			bt::BEncoder enc(&fptr);
			
			enc.beginList();
			for (iterator i = begin();i != end();i++)
			{
				i->second->save(&enc);
			}
			enc.end();
		}
		catch (bt::Error & err)
		{
			bt::Out() << "Error : " << err.toString() << endl;
			return;
		}
	}
	

		
	void GroupManager::loadGroups(const QString & fn)
	{
		bt::File fptr;
		if (!fptr.open(fn,"rb"))
		{
			bt::Out() << "Cannot open " << fn << " : " << fptr.errorString() << bt::endl;
			return;
		}	
		try
		{
			Uint32 fs = bt::FileSize(fn);
			QByteArray data(fs);
			fptr.read(data.data(),fs);
			
			BDecoder dec(data,false);
			bt::BNode* n = dec.decode();
			if (!n || n->getType() != bt::BNode::LIST)
				throw bt::Error("groups file corrupt");
			
			BListNode* ln = (BListNode*)n;
			for (Uint32 i = 0;i < ln->getNumChildren();i++)
			{
				BDictNode* dn = ln->getDict(i);
				if (!dn)
					continue;
				
				TorrentGroup* g = new TorrentGroup("dummy");
				
				try
				{
					g->load(dn);
				}
				catch (...)
				{
					delete g;
					throw;
				}
				
				if (!find(g->groupName()))
					insert(g->groupName(),g);
				else
					delete g;
			}
		}
		catch (bt::Error & err)
		{
			bt::Out() << "Error : " << err.toString() << endl;
			return;
		}
	}
	
	void GroupManager::torrentRemoved(TorrentInterface* ti)
	{
		for (iterator i = begin(); i != end();i++)
		{
			i->second->torrentRemoved(ti);
		}
	}
	
	void GroupManager::renameGroup(const QString & old_name,const QString & new_name)
	{
		Group* g = find(old_name);
		if (!g)
			return;
		
		setAutoDelete(false);
		erase(old_name);
		g->rename(new_name);
		insert(new_name,g);
		setAutoDelete(true);
	}
	
	Group* GroupManager::findDefault(const QString & name)
	{
		return default_groups.find(name);
	}
}
