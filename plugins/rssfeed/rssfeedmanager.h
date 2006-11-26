/***************************************************************************
 *   Copyright (C) 2006 by Alan Jones   				   *
 *   skyphyr@gmail.com   						   *
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
#ifndef RSSFEEDMANAGER_H
#define RSSFEEDMANAGER_H

#include <kdirlister.h>
#include <kfileitem.h>
#include <qstring.h>
#include <qobject.h>
#include <qdir.h>

#include <qptrlist.h>
#include <qwidget.h>
#include "rssfeedwidget.h"

#include "rssfeed.h"
#include "rssfilter.h"

namespace kt
{
	
	class CoreInterface;

	/**
	 * @brief RssFeed Manager Class
	 * @author Alan Jones <skyphyr@gmail.com>
	 * 
	 * 
	*/
	class RssFeedManager : public RssFeedWidget
	{
			Q_OBJECT
		public:
			
			/**
			 * Default constructor.
			 * @param core Pointer to core interface
			 * @param openSilently Wheather to open torrent silently or nor.
			 */
			RssFeedManager(CoreInterface* core, QWidget * parent = 0);
			~RssFeedManager();

		public slots:
			void changedActiveFeed();
			void changedArticleSelection();
			void changedFeedUrl();
			void changedMatchSelection();
			void updateArticles(const RssArticle::List& articles);
			void downloadSelectedArticles();
			void downloadSelectedMatches();
			void deleteSelectedMatches();
			
			void changedActiveAcceptFilter();
			void changedActiveRejectFilter();
			
			void clearArticles();
			
			void updateFeedList(int item=-1);
			void addNewFeed(RssFeed feed = RssFeed());
			void deleteSelectedFeed();
			
			void updateAcceptFilterList(int item=-1);
			void addNewAcceptFilter(RssFilter filter = RssFilter());
			void deleteSelectedAcceptFilter();
			
			void updateRejectFilterList(int item=-1);
			void addNewRejectFilter(RssFilter filter = RssFilter());
			void deleteSelectedRejectFilter();
			
			void updateRegExps();
			void updateMatches(const QValueList<FilterMatch>& matches);
			
			void saveFeedList();
			void saveFilterList();
			
			void disconnectFeed(int index);
			void connectFeed(int index);
			
			void disconnectFilter(int index, bool acceptFilter);
			void connectFilter(int index, bool acceptFilter);
			
			void scanArticle(RssArticle article, RssFilter * filter = NULL);
			void rescanFilter();
			
			void testTextChanged();
			void testFilter();
			
			void setFilterTitle(const QString& title);
			void setFeedTitle(const QString& title);

		private:
			CoreInterface* m_core;
			
			QPtrList<RssFeed> feeds;
			int currentFeed;
			
			QPtrList<RssFilter> acceptFilters;
			int currentAcceptFilter;
			QPtrList<RssFilter> rejectFilters;
			int currentRejectFilter;
			
			QString getFeedListFilename();
			void loadFeedList();
			
			QString getFilterListFilename();
			void loadFilterList();
			
			bool feedListSaving;
			bool filterListSaving;

	};
}
#endif
