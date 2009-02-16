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
#include "feedretriever.h"

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
		save();
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
			enc.write(f->filterID());
		enc.end();
		enc.write("loaded");
		enc.beginList();
		foreach (const QString & id,loaded)
			enc.write(id);
		enc.end();
		enc.write("downloaded_se_items");
		enc.beginList();
		QMap<Filter*,QList<SeasonEpisodeItem> >::iterator i = downloaded_se_items.begin();
		while (i != downloaded_se_items.end())
		{
			Filter* f = i.key();
			QList<SeasonEpisodeItem> & se = i.value();
			enc.write(f->filterID());
			enc.beginList();
			foreach (const SeasonEpisodeItem & item,se)
			{
				enc.write((bt::Uint32)item.season);
				enc.write((bt::Uint32)item.episode);
			}
			enc.end();
			i++;
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
		
		QByteArray data = fptr.readAll();
		BDecoder dec(data,false);
		BNode* n = dec.decode();
		if (!n || n->getType() != BNode::DICT)
		{
			delete n;
			return;
		}
		
		BDictNode* dict = (BDictNode*)n;
		
		try
		{
			url = KUrl(dict->getString("url",0));
			
			BListNode* fl = dict->getList("filters");
			if (fl)
			{
				for (Uint32 i = 0;i < fl->getNumChildren();i++)
				{
					Filter* f = filter_list->filterByID(fl->getString(i,0));
					if (f)
						filters.append(f);
				}
			}
			
			BListNode* ll = dict->getList("loaded");
			if (ll)
			{
				for (Uint32 i = 0;i < ll->getNumChildren();i++)
				{
					loaded.append(ll->getString(i,0));
				}
			}
			
			BListNode* se_list = dict->getList("downloaded_se_items");
			if (se_list)
			{
				for (Uint32 i = 0;i < se_list->getNumChildren();i+=2)
				{
					BListNode* se = se_list->getList(i+1);
					if (!se)
						continue;
					
					Filter* f = filter_list->filterByID(se_list->getString(i,0));
					if (!f)
						continue;
					
					QList<SeasonEpisodeItem> & sel = downloaded_se_items[f];
					for (Uint32 j = 0;j < se->getNumChildren();j+=2)
					{
						SeasonEpisodeItem item;
						item.episode = se->getInt(j);
						item.season = se->getInt(j+1);
						sel.append(item);
					}
				}
			}
		}
		catch (...)
		{
			delete n;
			throw;
		}
		
		Out(SYS_SYN|LOG_DEBUG) << "Loaded feed from " << file << " : " << endl;
		status = OK;
		delete n;
		
		if (bt::Exists(dir + "feed.xml"))
			loadFromDisk();
		else
			refresh();
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
		checkLoaded();
		runFilters();
		updated();
	}
	
	void Feed::refresh()
	{
		status = DOWNLOADING;
		update_timer.stop();
		Syndication::Loader *loader = Syndication::Loader::create(this,SLOT(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
		loader->loadFrom(url,new FeedRetriever(dir + "feed.xml"));
		updated();
	}
	
	void Feed::loadingFromDiskComplete(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode status)
	{
		loadingComplete(loader,feed,status);
		refresh();
	}
	
	void Feed::loadFromDisk()
	{
		status = DOWNLOADING;
		update_timer.stop();
		Syndication::Loader *loader = Syndication::Loader::create(this,SLOT(loadingFromDiskComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
		loader->loadFrom(KUrl(dir + "feed.xml"));
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
	
	bool Feed::needToDownload(Syndication::ItemPtr item,Filter* filter)
	{
		bool m = filter->match(item);
		if ((m && filter->downloadMatching()) || (!m && filter->downloadNonMatching()))
		{
			if (filter->useSeasonAndEpisodeMatching() && filter->noDuplicateSeasonAndEpisodeMatches())
			{
				int s = 0;
				int e = 0;
				Filter::getSeasonAndEpisode(item->title(),s,e);
				if (!downloaded_se_items.contains(filter))
				{
					downloaded_se_items[filter].append(SeasonEpisodeItem(s,e));
				}
				else
				{
					// If we have already downloaded this season and episode, return
					QList<SeasonEpisodeItem> & ses = downloaded_se_items[filter];
					SeasonEpisodeItem se(s,e);
					if (ses.contains(se))
						return false;
					
					ses.append(se);
				}
			}
			
			return true;
		}
		
		return false;
	}
		
	void Feed::runFilters()
	{
		if (!feed)
			return;
		
		Out(SYS_SYN|LOG_NOTICE) << "Running filters on " << feed->title() << endl;
		foreach (Filter* f,filters)
		{
			f->startMatching();
			QList<Syndication::ItemPtr> items = feed->items();
			foreach (Syndication::ItemPtr item,items)
			{
				// Skip already loaded items
				if (loaded.contains(item->id()))
					continue;
				
				if (needToDownload(item,f))
				{
					Out(SYS_SYN|LOG_NOTICE) << "Downloading item " << item->title() << " (filter: " << f->filterName() << ")" << endl;
					downloadItem(item,f->group(),f->downloadLocation(),f->openSilently());
				}
			}
		}
	}
	
	QString TorrentUrlFromItem(Syndication::ItemPtr item);
	
	void Feed::downloadItem(Syndication::ItemPtr item,const QString & group,const QString & location,bool silently)
	{
		loaded.append(item->id());
		QString url = TorrentUrlFromItem(item);
		if (!url.isEmpty())
			downloadLink(KUrl(url),group,location,silently);
		else
			downloadLink(KUrl(item->link()),group,location,silently);
		save();
	}
		
	void Feed::clearFilters()
	{
		filters.clear();
	}
	
	void Feed::checkLoaded()
	{
		// remove all id's which are in loaded but no longer in 
		// the item list
		bool need_to_save = false;
		QList<Syndication::ItemPtr> items = feed->items();
		for (QStringList::iterator i = loaded.begin();i != loaded.end();)
		{
			bool found = false;
			foreach (Syndication::ItemPtr item,items)
			{
				if (item->id() == *i)
				{
					found = true;
					break;
				}
			}
			
			if (!found)
			{
				need_to_save = true;
				i = loaded.erase(i);
			}
			else
				i++;
		}
		
		if (need_to_save)
			save();
	}
	
	bool  Feed::downloaded(Syndication::ItemPtr item) const
	{
		return loaded.contains(item->id());
	}
	
	//////////////////////////////////
	SeasonEpisodeItem::SeasonEpisodeItem() : season(-1),episode(-1)
	{}
	
	SeasonEpisodeItem::SeasonEpisodeItem(int s,int e) : season(s),episode(e)
	{}
	
	SeasonEpisodeItem::SeasonEpisodeItem(const SeasonEpisodeItem & item) : season(item.season),episode(item.episode)
	{}
	
	bool SeasonEpisodeItem::operator == (const SeasonEpisodeItem & item) const
	{
		return season == item.season && episode == item.episode;
	}
	
	SeasonEpisodeItem & SeasonEpisodeItem::operator = (const SeasonEpisodeItem & item)
	{
		season = item.season;
		episode = item.episode;
		return *this;
	}
}
