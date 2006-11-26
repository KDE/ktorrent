/***************************************************************************
 *   Copyright (C) 2006 by Alan Jones                                      *
 *   skyphyr@gmail.com                                                     *
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
#ifndef RSSFEED_H
#define RSSFEED_H

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <qdatastream.h>

#include <kurl.h>

#include "rss/loader.h"
#include "rss/document.h"

#include "rssarticle.h"

using namespace RSS;

namespace kt
{
	/**
	 * @brief RssFeed Class
	 * @author Alan Jones <skyphyr@gmail.com>
	 * 
	 * 
	*/
	
	class RssFeed : public QObject
	{
			Q_OBJECT
		public:
			
			/**
			 * Default constructor.
			 */
			RssFeed(QObject * parent = 0);
			RssFeed(KURL feedUrl, QString title = "", bool active = false, int articleAge = 3, bool ignoreTTL = false, QTime autoRefresh = QTime());
			RssFeed(const RssFeed &other);
 			RssFeed &operator=(const RssFeed &other);
 			~RssFeed();
 			
 			KURL feedUrl() const { return m_feedUrl; }
 			bool active() const { return m_active; }
 			int articleAge() const { return m_articleAge; }
 			QString title() const { return m_title; }
 			QTime autoRefresh() const { return m_autoRefresh; }
 			bool ignoreTTL() const { return m_ignoreTTL; }
 			
 			
 			RssArticle::List articles() const { return m_articles; }
 			

		public slots:
			void refreshFeed();
			void feedLoaded(Loader *feedLoader, Document doc, Status status);
			
			void clearArticles();
			
			void setFeedUrl( const KURL& url );
			void setFeedUrl( const QString& url );
			void setActive( bool active );
			void setArticleAge( int articleAge );
			void setTitle( const QString& title );
			void setAutoRefresh( const QTime& autoRefresh );
			void setIgnoreTTL( bool ignoreTTL );
			void saveArticles();
			
			void setDownloaded(QString link, int downloaded);
			
		signals:
			void feedUrlChanged( const KURL& url );
			void activeChanged( bool active );
			void articleAgeChanged( int articleAge );
			void titleChanged( const QString& title );
			void updateTitle( const QString& title );
			void autoRefreshChanged( const QTime& autoRefresh );
			void ignoreTTLChanged( bool ignoreTTL );
			
			void articlesChanged( const RssArticle::List& articles );
			
			void scanRssArticle( RssArticle article );

		private:
			KURL m_feedUrl;
			bool m_active;
			int m_articleAge;
			QString m_title;
			QTime m_autoRefresh;
			bool m_ignoreTTL;
			RssArticle::List m_articles;
			QTimer refreshTimer;
			
			bool feedLoading;
			
			QString getFilename();
			void initialize();
			void startFeed();
			void cleanArticles();
			void loadArticles();
	};

	QDataStream &operator<<( QDataStream &out, const RssFeed &feed );
	QDataStream &operator>>( QDataStream &in, RssFeed &feed );

}

#endif
