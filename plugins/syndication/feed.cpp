/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include <util/fileops.h>
#include <bcodec/bnode.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include "feed.h"
#include "filter.h"
#include "filterlist.h"

using namespace bt;

namespace kt
{
	Feed::Feed(const QString & dir) : dir(dir),status(UNLOADED)
	{
		connect(&update_timer,SIGNAL(timeout()),this,SLOT(refresh()));
	}
	
	Feed::Feed(const KUrl & url,const QString & dir) : url(url),dir(dir),status(UNLOADED)
	{
		connect(&update_timer,SIGNAL(timeout()),this,SLOT(refresh()));
		refresh();
	}
	
	Feed::Feed(const KUrl & url,Syndication::FeedPtr feed,const QString & dir) : url(url),feed(feed),dir(dir),status(OK)
	{
		connect(&update_timer,SIGNAL(timeout()),this,SLOT(refresh()));
		update_timer.start(3600 * 1000);
	}


	Feed::~Feed()
	{
	}
	
	void Feed::save()
	{
		QString file = dir + "info";
		File fptr;
		if (!fptr.open(file,"wt"))
		{
			Out(SYS_SYN|LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
			return;
		}
		
		BEncoder enc(&fptr);
		enc.beginDict();
		enc.write("url");
		enc.write(url.prettyUrl());
		enc.write("filters");
		enc.beginList();
		foreach (Filter* f,filters)
		{
			enc.write(f->filterName());
		}
		enc.end();
		enc.end();
	}
	
	void Feed::load(FilterList* filter_list)
	{
		QString file = dir + "info";
		QFile fptr(file);
		if (!fptr.open(QIODevice::ReadOnly))
		{
			Out(SYS_SYN|LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
			return;
		}
		
		BDecoder dec(fptr.readAll(),false);
		BNode* n = dec.decode();
		if (!n || n->getType() != BNode::DICT)
		{
			delete n;
			return;
		}
		
		BDictNode* dict = (BDictNode*)n;
		BValueNode* vn = dict->getValue("url");
		if (!vn)
		{
			delete n;
			return;
		}
		
		url = KUrl(vn->data().toString());
		
		BListNode* fl = dict->getList("filters");
		if (fl)
		{
			for (Uint32 i = 0;i < fl->getNumChildren();i++)
			{
				vn = fl->getValue(i);
				if (!vn)
					continue;
					
				Filter* f = filter_list->filterByName(vn->data().toString());
				if (f)
					filters.append(f);
			}
		}
		Out(SYS_SYN|LOG_DEBUG) << "Loaded feed from " << file << " : " << endl;
		status = OK;
		delete n;
	}

	void Feed::loadingComplete(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode status)
	{
		Q_UNUSED(loader);
		
		if (status != Syndication::Success)
		{
			Out(SYS_SYN|LOG_NOTICE) << "Failed to load feed " << url.prettyUrl() << endl;
			this->status = FAILED_TO_DOWNLOAD;
			update_timer.start(3600 * 1000);
			updated();
			return;
		}
		
		Out(SYS_SYN|LOG_NOTICE) << "Loaded feed " << url.prettyUrl() << endl;
		this->feed = feed;
		update_timer.start(3600 * 1000);
		this->status = OK;
		updated();
	}
	
	void Feed::refresh()
	{
		status = DOWNLOADING;
		update_timer.stop();
		Syndication::Loader *loader = Syndication::Loader::create(this,SLOT(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
		loader->loadFrom(url);
		updated();
	}
	
	QString Feed::title() const
	{
		if (feed)
			return feed->title();
		else
			return url.prettyUrl();
	}
	
	QString Feed::newFeedDir(const QString & base)
	{
		int cnt = 0;
		QString dir = QString("%1feed%2/").arg(base).arg(cnt);
		while (bt::Exists(dir))
		{
			cnt++;
			dir = QString("%1feed%2/").arg(base).arg(cnt);
		}
		
		bt::MakeDir(dir);
		return dir;
	}
	
	void Feed::addFilter(Filter* f)
	{
		filters.append(f);
	}
		
	void Feed::removeFilter(Filter* f)
	{
		filters.removeAll(f);
	}
		
	void Feed::runFilters()
	{
		
	}
		
	void Feed::clearFilters()
	{
		filters.clear();
	}
}
