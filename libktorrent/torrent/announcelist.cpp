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
#if 0 
#include "announcelist.h"
#include "bnode.h"
#include <util/error.h>
#include "globals.h"
#include <util/log.h>

#include <klocale.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qtextstream.h>

namespace bt
{

	AnnounceList::AnnounceList()
		:m_datadir(QString::null)
	{
		curr = 0;
	}


	AnnounceList::~AnnounceList()
	{
		saveTrackers();
	}

	void AnnounceList::load(BNode* node)
	{
		BListNode* ml = dynamic_cast<BListNode*>(node);
		if (!ml)
			return;
		
		//ml->printDebugInfo();
		for (Uint32 i = 0;i < ml->getNumChildren();i++)
		{
			BListNode* url = dynamic_cast<BListNode*>(ml->getChild(i));
			if (!url)
				throw Error(i18n("Parse Error"));
			
			for (Uint32 j = 0;j < url->getNumChildren();j++)
			{
				BValueNode* vn = dynamic_cast<BValueNode*>(url->getChild(j));
				if (!vn)
					throw Error(i18n("Parse Error"));

				KURL url(vn->data().toString().stripWhiteSpace());
				trackers.append(url);
				//Out() << "Added tracker " << url << endl;
			}
		}
	}
	
	const KURL::List AnnounceList::getTrackerURLs()
	{
		KURL::List complete(trackers);
		complete += custom_trackers;
		return complete;
	}
	
	void AnnounceList::addTracker(KURL url, bool custom)
	{
		if(custom)
			custom_trackers.append(url);
		else
			trackers.append(url);
	}
	
	bool AnnounceList::removeTracker(KURL url)
	{
		KURL::List::iterator i = custom_trackers.find(url);
		if(i != custom_trackers.end())
		{
			custom_trackers.remove(i);
			return true;
		}
		else
			return false;
	}
	
	KURL AnnounceList::getTrackerURL(bool last_was_succesfull) const
	{
		int defaults = trackers.count();
		int customs = custom_trackers.count();
		int total = defaults + customs;
		
		if (total == 0)
			return KURL(); // return invalid url is there are no trackers
		
		if (last_was_succesfull)
			return curr < defaults ? *trackers.at(curr) : *custom_trackers.at(curr % customs);
		
		curr = (curr + 1) % total;
		return curr < defaults ? *trackers.at(curr) : *custom_trackers.at(curr % customs);
	}

	void AnnounceList::debugPrintURLList()
	{
		Out() << "Announce List : " << endl;
		for (KURL::List::iterator i = trackers.begin();i != trackers.end();i++)
			Out() << "URL : " << *i << endl;
	}
	
	void AnnounceList::saveTrackers()
	{
		QFile file(m_datadir + "trackers");
		if(!file.open(IO_WriteOnly))
			return;
		
		QTextStream stream(&file);
		for (KURL::List::iterator i = custom_trackers.begin();i != custom_trackers.end();i++)
			stream << (*i).prettyURL() << ::endl;
		file.close();
	}
	
	void AnnounceList::loadTrackers()
	{
		QFile file(m_datadir + "trackers");
		if(!file.open(IO_ReadOnly))
			return;
		
		QTextStream stream(&file);
		while (!stream.atEnd()) 
		{
			KURL url(stream.readLine().stripWhiteSpace());
			custom_trackers.append(url);
		}
		
		file.close();
	}
	
	void AnnounceList::setDatadir(const QString& theValue)
	{
		m_datadir = theValue;
		loadTrackers();
	}
	
	void AnnounceList::setTracker(KURL url)
	{
		int defaults = trackers.count();
		int customs = custom_trackers.count();
		int total = defaults + customs;
		
		int backup = curr;
		
		for(curr=0; curr<defaults; ++curr)
		{
			if( *trackers.at(curr) == url )
				return;
		}
		
		for( ; curr<total; ++curr)
		{
			if( *custom_trackers.at(curr % customs) == url )
				return;
		}
		
		curr = backup;
	}
	
	void AnnounceList::restoreDefault()
	{
		curr = 0;
	}

	void AnnounceList::merge(const AnnounceList* al)
	{
		for (Uint32 i = 0;i < al->getNumTrackerURLs();i++)
		{
			KURL url = *al->trackers.at(i);
			if (!trackers.contains(url) && !custom_trackers.contains(url))
				custom_trackers.append(url);
		}
	}
}
#endif
