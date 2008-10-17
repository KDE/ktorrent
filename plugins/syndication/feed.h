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
#ifndef KTFEED_H
#define KTFEED_H

#include <QTimer>
#include <kurl.h>
#include <syndication/feed.h>
#include <syndication/loader.h>

namespace kt
{

	/**
		Class to keep track of a feed.
	*/
	class Feed : public QObject
	{
		Q_OBJECT
	public:
		Feed(const QString & dir);
		Feed(const KUrl & url,const QString & dir);
		Feed(const KUrl & url,Syndication::FeedPtr feed,const QString & dir);
		virtual ~Feed();
		
		enum Status
		{
			UNLOADED, OK, FAILED_TO_DOWNLOAD, DOWNLOADING
		};
		
		/// Get the libsyndication feed 
		Syndication::FeedPtr feedData() {return feed;}
		
		/// Get the URL of the feed
		KUrl feedUrl() const {return url;}
		
		/// Is the feed OK
		bool ok() const {return feed.get() != 0;}
		
		/// Save the feed to it's directory
		void save();
		
		/// Load the feed from it's directory
		void load();
		
		/// Get the feed's data directory
		QString directory() const {return dir;}
		
		/// Get the current status of the feed
		Status feedStatus() const {return status;}
		
		/// Get the tile of the feed
		QString title() const;
		
		/// Create a new feed directory
		static QString newFeedDir(const QString & base);
		
	public slots:
		/// Update the feed
		void refresh();
		
	private slots:
		void loadingComplete(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode status);
		
	signals:
		void updated();

	private:
		KUrl url;
		Syndication::FeedPtr feed;
		QString dir;
		QTimer update_timer;
		Status status;
	};

}

#endif
