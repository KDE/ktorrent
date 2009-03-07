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
#include <QFile>
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/sha1hash.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bnode.h>
#include <torrent/queuemanager.h>
#include "shutdownruleset.h"


using namespace bt;

namespace kt
{
	
	ShutdownRuleSet::ShutdownRuleSet(kt::CoreInterface* core, QObject* parent) : QObject(parent),core(core),on(false)
	{
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),this,SLOT(torrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),this,SLOT(torrentRemoved(bt::TorrentInterface*)));
		QueueManager* qman = core->getQueueManager();
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			torrentAdded(*i);
		}
		
		// Add the default rule
		addRule(SHUTDOWN,ALL_TORRENTS,DOWNLOADING_COMPLETED);
	}
	
	ShutdownRuleSet::~ShutdownRuleSet() 
	{
	}

	void ShutdownRuleSet::clear() 
	{
		rules.clear();
	}

	void ShutdownRuleSet::addRule(kt::Action action, kt::Target target, kt::Trigger trigger,bt::TorrentInterface* tc) 
	{
		ShutdownRule rule;
		rule.action = action;
		rule.target = target;
		rule.trigger = trigger;
		rule.tc = tc;
		rule.hit = false;
		rules.append(rule);
	}
	
	void ShutdownRuleSet::torrentFinished(bt::TorrentInterface* tc) 
	{
		if (!on)
			return;
		
		for (QList<ShutdownRule>::iterator i = rules.begin();i != rules.end();i++)
		{
			if (i->downloadingFinished(tc,core->getQueueManager()))
			{
				// Rule has been hit now emit the correct signal
				switch (i->action)
				{
					case SHUTDOWN: emit shutdown(); break;
					case LOCK: emit lock(); break;
					case STANDBY: emit standby(); break;
					case SUSPEND_TO_DISK: emit suspendToDisk(); break;
					case SUSPEND_TO_RAM: emit suspendToRAM(); break;
				}
				break;
			}
		}
	}
	
	void ShutdownRuleSet::seedingAutoStopped(bt::TorrentInterface* tc, bt::AutoStopReason reason) 
	{
		Q_UNUSED(reason);
		if (!on)
			return;
		
		for (QList<ShutdownRule>::iterator i = rules.begin();i != rules.end();i++)
		{
			if (i->seedingFinished(tc,core->getQueueManager()))
			{
				// Rule has been hit now emit the correct signal
				switch (i->action)
				{
					case SHUTDOWN: emit shutdown(); break;
					case LOCK: emit lock(); break;
					case STANDBY: emit standby(); break;
					case SUSPEND_TO_DISK: emit suspendToDisk(); break;
					case SUSPEND_TO_RAM: emit suspendToRAM(); break;
				}
				break;
			}
		}
	}
	
	void ShutdownRuleSet::torrentAdded(bt::TorrentInterface* tc) 
	{
		connect(tc,SIGNAL(seedingAutoStopped(bt::TorrentInterface*,bt::AutoStopReason)),
				this,SLOT(seedingAutoStopped(bt::TorrentInterface*,bt::AutoStopReason)));
		connect(tc,SIGNAL(finished(bt::TorrentInterface*)),this,SLOT(torrentFinished(bt::TorrentInterface*)));
	}


	void ShutdownRuleSet::torrentRemoved(bt::TorrentInterface* tc) 
	{
		// Throw away all rules for this torrent
		for (QList<ShutdownRule>::iterator i = rules.begin();i != rules.end();)
		{
			if (i->tc == tc)
				i = rules.erase(i);
			else
				i++;
		}
	}
	
	void ShutdownRuleSet::setEnabled(bool on) 
	{
		this->on = on;
	}
	
	void ShutdownRuleSet::save(const QString& file) 
	{
		File fptr;
		if (!fptr.open(file,"wt"))
		{
			Out(SYS_GEN|LOG_DEBUG) << "Failed to open file " << file << " : " << fptr.errorString() << endl;
			return;
		}
		
		BEncoder enc(new BEncoderFileOutput(&fptr));
		enc.beginList();
		for (QList<ShutdownRule>::iterator i = rules.begin();i != rules.end();i++)
		{
			enc.beginDict();
			enc.write("Action",(bt::Uint32)i->action);
			enc.write("Trigger",(bt::Uint32)i->trigger);
			enc.write("Target",(bt::Uint32)i->target);
			if (i->target == SPECIFIC_TORRENT)
			{
				bt::SHA1Hash hash = i->tc->getInfoHash();
				enc.write("Torrent");
				enc.write(hash.getData(),20);
			}
			enc.end();
		}
		enc.write(on);
		enc.end();
	}
	
	void ShutdownRuleSet::load(const QString& file) 
	{
		QFile fptr(file);
		if (!fptr.open(QIODevice::ReadOnly))
		{
			Out(SYS_GEN|LOG_DEBUG) << "Failed to open file " << file << " : " << fptr.errorString() << endl;
			return;
		}
		
		QByteArray data = fptr.readAll();
		BDecoder dec(data,false);
		BNode* node = 0;
		try 
		{
			clear();
			node = dec.decode();
			if (node->getType() != BNode::LIST)
				throw bt::Error("Toplevel node not a list");
			
			BListNode* l = (BListNode*)node;
			for (int i = 0;i < l->getNumChildren() - 1;i++)
			{
				BDictNode* d = l->getDict(i);
				ShutdownRule rule;
				rule.action = (Action)d->getInt("Action");
				rule.target = (Target)d->getInt("Target");
				rule.trigger = (Trigger)d->getInt("Trigger");
				rule.tc = 0;
				if (d->getValue("Torrent"))
				{
					QByteArray hash = d->getByteArray("Torrent");
					bt::TorrentInterface* tc = torrentForHash(hash);
					if (tc)
						rule.tc = tc;
					else
						continue; // no valid torrent found so skip this rule
				}
				rules.append(rule);
			}
			
			on = l->getInt(l->getNumChildren() - 1) == 1;
		}
		catch (bt::Error & err)
		{
			Out(SYS_GEN|LOG_DEBUG) << "Failed to parse " << file << " : " << err.toString() << endl;
			addRule(SHUTDOWN,ALL_TORRENTS,DOWNLOADING_COMPLETED);
		}
		
		delete node;
	}
	
	bt::TorrentInterface* ShutdownRuleSet::torrentForHash(const QByteArray& hash) 
	{
		bt::SHA1Hash ih((const bt::Uint8*)hash.data());
		QueueManager* qman = core->getQueueManager();
		for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
		{
			bt::TorrentInterface* t = *i;
			if (t->getInfoHash() == ih)
				return t;
		}
		
		return 0;
	}
	
	kt::Action ShutdownRuleSet::currentAction() const 
	{
		if (rules.count() == 0)
			return SHUTDOWN;
		else
			return rules.front().action;
	}


	
	//////////////////////////////////////
	
	bool ShutdownRule::downloadingFinished(bt::TorrentInterface* tor,QueueManager* qman) 
	{
		if (target != ALL_TORRENTS && tc != tor)
			return false;
		
		if (trigger != DOWNLOADING_COMPLETED)
			return false;
		
		if (target != ALL_TORRENTS)
		{
			hit = tc == tor;
			return hit;
		}
		else
		{
			// target is all torrents, so check if all torrents have completed downloading
			for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			{
				bt::TorrentInterface* t = *i;
				const bt::TorrentStats & stats = t->getStats();
				if (t != tor && !stats.completed && stats.running)
					return false;
			}
			
			hit = true;
			return true;
		}
	}
	
	bool ShutdownRule::seedingFinished(bt::TorrentInterface* tor,QueueManager* qman) 
	{
		if (target != ALL_TORRENTS && tc != tor)
			return false;
		
		if (trigger != SEEDING_COMPLETED)
			return false;
		
		if (target != ALL_TORRENTS)
		{
			hit = tc == tor;
			return hit;
		}
		else
		{
			// target is all torrents, so check if all torrents have completed seeding
			for (QueueManager::iterator i = qman->begin();i != qman->end();i++)
			{
				bt::TorrentInterface* t = *i;
				if (t == tor)
					continue;
				
				const bt::TorrentStats & stats = t->getStats();
				if (stats.running)
					return false;
			}
			
			hit = true;
			return true;
		}
	}


}

