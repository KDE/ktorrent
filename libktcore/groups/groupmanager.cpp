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
#include <bcodec/bnode.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <interfaces/functions.h>
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
#include "ungroupedgroup.h"

using namespace bt;

namespace kt
{

	GroupManager::GroupManager()
	{
		setAutoDelete(true);
		
		all = new AllGroup();
		upload = new UploadGroup();
		download = new DownloadGroup();
		queued_downloads = new QueuedDownloadsGroup();
		queued_uploads = new QueuedUploadsGroup();
		user_downloads = new UserDownloadsGroup();
		user_uploads = new UserUploadsGroup();
		active = new ActiveGroup();
		active_uploads = new ActiveUploadsGroup();
		active_downloads = new ActiveDownloadsGroup();
		inactive = new InactiveGroup();
		inactive_uploads = new InactiveUploadsGroup();
		inactive_downloads = new InactiveDownloadsGroup();
		ungrouped = new UngroupedGroup(this);
	}


	GroupManager::~GroupManager()
	{
		delete all;
		delete download;
		delete upload;
		delete queued_downloads;
		delete queued_uploads;
		delete user_downloads;
		delete user_uploads;
		delete inactive;
		delete inactive_downloads;
		delete inactive_uploads;
		delete active;
		delete active_downloads;
		delete active_uploads;
		delete ungrouped;
	}


	Group* GroupManager::newGroup(const QString & name)
	{
		if (find(name))
			return 0;
		
		Group* g = new TorrentGroup(name);
		insert(name,g);
		emit customGroupsChanged();
		return g;
	}
	
	bool GroupManager::canRemove(const Group* g) const
	{
		return find(g->groupName()) != 0;
	}
	
	bool GroupManager::erase(const QString & key)
	{
		bt::PtrMap<QString,Group>::erase(key);
		emit customGroupsChanged();
	}
	
	Group* GroupManager::findDefault(const QString & name)
	{
		QList<Group*> def;
		def << all << download << upload << queued_downloads << queued_uploads << user_downloads
				<< user_uploads << inactive << inactive_downloads << inactive_uploads << active 
				<< active_downloads << active_uploads << ungrouped;
		
		foreach (Group* g,def)
		{
			if (g->groupName() == name)
				return g;
		}
		return 0;
	}
	
	QStringList GroupManager::customGroupNames()
	{
		QStringList groupNames;
		iterator it = begin();
		
		while(it != end())
		{
			groupNames << it->first;
			++it;
		}
		
		return groupNames;
	}
	
	void GroupManager::saveGroups()
	{
		QString fn = kt::DataDir() + "groups";
		bt::File fptr;
		if (!fptr.open(fn,"wb"))
		{
			bt::Out(SYS_GEN|LOG_DEBUG) << "Cannot open " << fn << " : " << fptr.errorString() << bt::endl;
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
			bt::Out(SYS_GEN|LOG_DEBUG) << "Error : " << err.toString() << endl;
			return;
		}
	}
	

		
	void GroupManager::loadGroups()
	{
		QString fn = kt::DataDir() + "groups";
		bt::File fptr;
		if (!fptr.open(fn,"rb"))
		{
			bt::Out(SYS_GEN|LOG_DEBUG) << "Cannot open " << fn << " : " << fptr.errorString() << bt::endl;
			return;
		}	
		try
		{
			Uint32 fs = bt::FileSize(fn);
			QByteArray data(fs,0);
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
			bt::Out(SYS_GEN|LOG_DEBUG) << "Error : " << err.toString() << endl;
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
		QString oldName = old_name;
		Group* g = find(old_name);
		if (!g)
			return;
		
		setAutoDelete(false);
		bt::PtrMap<QString,Group>::erase(old_name);
		g->rename(new_name);
		insert(new_name,g);
		setAutoDelete(true);
		saveGroups();
		
		emit customGroupsChanged(oldName, new_name);
	}
}
