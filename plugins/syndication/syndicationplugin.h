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
#ifndef KTSYNDICATIONPLUGIN_H
#define KTSYNDICATIONPLUGIN_H

#include <interfaces/plugin.h>
#include <syndication/loader.h>
#include <interfaces/guiinterface.h>

class KAction;

namespace kt
{
	class Feed;
	class FeedList;
	class SyndicationTab;
	class FeedWidget;
	class Filter;
	class FilterList;

	/**
		@author
	*/
	class SyndicationPlugin : public Plugin,public CloseTabListener
	{
		Q_OBJECT
	public:
		SyndicationPlugin(QObject* parent,const QStringList& args);
		virtual ~SyndicationPlugin();

		virtual bool versionCheck(const QString& version) const;
		virtual void load();
		virtual void unload();
		Filter* addNewFilter();
		
	private slots:
		void addFeed();
		void removeFeed();
		void loadingComplete(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode status);
		void activateFeedWidget(Feed* f);
		void downloadLink(const KUrl & url,const QString & group,const QString & location,bool silently);
		void updateTabText(QWidget* w,const QString & text);
		void showFeed();
		void addFilter();
		void removeFilter();
		void editFilter();
		void editFilter(Filter* f);
		void manageFilters();
						
	private:
		void setupActions();
		void loadTabs();
		virtual void tabCloseRequest(kt::GUIInterface* gui, QWidget* tab);
		
	private:
		KAction* add_feed;
		KAction* remove_feed;
		KAction* show_feed;
		KAction* add_filter;
		KAction* remove_filter;
		KAction* edit_filter;
		KAction* manage_filters;
		FeedList* feed_list;
		FilterList* filter_list;
		SyndicationTab* tab;
		QMap<Syndication::Loader*,KUrl> downloads;
		QMap<Feed*,FeedWidget*> tabs;
	};

}

#endif
