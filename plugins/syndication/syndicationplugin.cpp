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
#include <kurl.h>
#include <kgenericfactory.h>
#include <kactioncollection.h>
#include <kinputdialog.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <util/log.h>
#include <util/logsystemmanager.h>
#include <util/fileops.h>
#include <util/error.h>
#include <interfaces/functions.h>
#include <interfaces/guiinterface.h>
#include "syndicationplugin.h"
#include "feed.h"
#include "feedlist.h"
#include "feedlistview.h"
#include "feedwidget.h"
#include "linkdownloader.h"

K_EXPORT_COMPONENT_FACTORY(ktsyndicationplugin,KGenericFactory<kt::SyndicationPlugin>("ktsyndicationplugin"))
		
using namespace bt;

namespace kt
{

	SyndicationPlugin::SyndicationPlugin(QObject* parent,const QStringList& args): Plugin(parent),add_feed(0)
	{
		Q_UNUSED(args);
		feed_view = 0;
		feed_list = 0;
		setupActions();
	//	setXMLFile("ktsyndicationpluginui.rc");
		LogSystemManager::instance().registerSystem(i18n("Syndication"),SYS_SYN);
	}


	SyndicationPlugin::~SyndicationPlugin()
	{
		LogSystemManager::instance().unregisterSystem(i18n("Syndication"));
	}


	bool SyndicationPlugin::versionCheck(const QString& version) const
	{
		return version == KT_VERSION_MACRO;
	}

	void SyndicationPlugin::load()
	{
		QString ddir = kt::DataDir() + "syndication/";
		if (!bt::Exists(ddir))
			bt::MakeDir(ddir,true);
		
		feed_list = new FeedList(ddir,this);
		feed_view = new FeedListView(actionCollection(),feed_list,0);
		connect(feed_view,SIGNAL(feedActivated(Feed*)),this,SLOT(activateFeedWidget(Feed*)));
		connect(feed_view,SIGNAL(enableRemove(bool)),remove_feed,SLOT(setEnabled(bool)));
		connect(feed_view,SIGNAL(enableRemove(bool)),show_feed,SLOT(setEnabled(bool)));
		getGUI()->addToolWidget(feed_view,"application-rss+xml",i18n("Feeds"),GUIInterface::DOCK_LEFT);
		feed_list->loadFeeds();
		loadTabs();
	}
	
	void SyndicationPlugin::loadTabs()
	{
		KConfigGroup g = KGlobal::config()->group("SyndicationTabs");
		QStringList tabs;
		tabs = g.readEntry("tabs",tabs);
		foreach (const QString & tab,tabs)
		{
			Feed* f = feed_list->feedForDirectory(tab);
			if (f)
				activateFeedWidget(f);
		}
	}

	void SyndicationPlugin::unload()
	{
		QStringList active_tabs;
		QMap<Feed*,FeedWidget*>::iterator i = tabs.begin();
		while (i != tabs.end())
		{
			Feed* f = i.key();
			active_tabs << f->directory();
			getGUI()->removeTabPage(i.value());
			i.value()->deleteLater();
			i++;
		}
		tabs.clear();
		
		KConfigGroup g = KGlobal::config()->group("SyndicationTabs");
		g.writeEntry("tabs",active_tabs);
		g.sync();
		
		getGUI()->removeToolWidget(feed_view);
		delete feed_view;
		delete feed_list;
		feed_view = 0;
		feed_list = 0;
	}
	
	void SyndicationPlugin::addFeed()
	{
		bool ok = false;
		QString url = KInputDialog::getText(
						i18n("Enter the URL"),
						i18n("Please enter the URL of the RSS or Atom feed."),
						QString(),&ok,getGUI()->getMainWindow());
		if (!ok || url.isEmpty())
			return;
		
		Syndication::Loader *loader = Syndication::Loader::create(this,SLOT(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
		loader->loadFrom(KUrl(url));
		downloads.insert(loader,KUrl(url));
	}
	
	void SyndicationPlugin::loadingComplete(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode status)
	{
		if (status != Syndication::Success)
		{
			KMessageBox::error(feed_view,i18n("Failed to load feed %1 !",downloads[loader].prettyUrl()));
			downloads.remove(loader);
			return;
		}
		
		try
		{
			QString ddir = kt::DataDir() + "syndication/";
			Feed* f = new Feed(downloads[loader],feed,Feed::newFeedDir(ddir));
			f->save();
			feed_list->addFeed(f);
		}
		catch (bt::Error & err)
		{
			KMessageBox::error(feed_view,i18n("Failed to create directory for feed %1 : %2",downloads[loader].prettyUrl(),err.toString()));
		}
		downloads.remove(loader);
	}
	
	void SyndicationPlugin::removeFeed()
	{
		QModelIndexList idx = feed_view->selectedFeeds();
		foreach (const QModelIndex & i,idx)
		{
			Feed* f = feed_list->feedForIndex(i);
			// close tab window if one is open for this feed
			if (f && tabs.contains(f))
			{
				QMap<Feed*,FeedWidget*>::iterator itr = tabs.find(f);
				getGUI()->removeTabPage(itr.value());
				itr.value()->deleteLater();
				tabs.erase(itr);
			}
		}
		feed_list->removeFeeds(idx);
	}
		
	void SyndicationPlugin::setupActions()
	{
		KActionCollection* ac = actionCollection();
		
		add_feed = new KAction(KIcon("list-add"),i18n("Add Feed"),this);
		connect(add_feed,SIGNAL(triggered()),this,SLOT(addFeed()));
		ac->addAction("add_feed",add_feed);
		
		remove_feed = new KAction(KIcon("list-remove"),i18n("Remove Feed"),this);
		connect(remove_feed,SIGNAL(triggered()),this,SLOT(removeFeed()));
		ac->addAction("remove_feed",remove_feed);
		
		show_feed = new KAction(KIcon("tab-new"),i18n("Show Feed"),this);
		connect(show_feed,SIGNAL(triggered()),this,SLOT(showFeed()));
		ac->addAction("show_feed",show_feed);
		
		remove_feed->setEnabled(false);
		show_feed->setEnabled(false);
	}
	
	void SyndicationPlugin::tabCloseRequest(kt::GUIInterface* gui, QWidget* tab)
	{
		QMap<Feed*,FeedWidget*>::iterator i = tabs.begin();
		while (i != tabs.end())
		{
			if (i.value() == tab)
			{
				getGUI()->removeTabPage(i.value());
				i.value()->deleteLater();
				tabs.erase(i);
				break;
			}
			i++;
		}
	}
	
	void SyndicationPlugin::activateFeedWidget(Feed* f)
	{
		if (!f)
			return;
		
		QMap<Feed*,FeedWidget*>::iterator i = tabs.find(f);
		if (i == tabs.end())
		{
			FeedWidget* fw = new FeedWidget(f,0);
			connect(fw,SIGNAL(downloadLink(const KUrl&)),this,SLOT(downloadLink(const KUrl&)));
			connect(fw,SIGNAL(updateCaption(QWidget*, const QString&)),this,SLOT(updateTabText(QWidget*, const QString&)));
			tabs.insert(f,fw);
			getGUI()->addTabPage(fw,"application-rss+xml",f->title(),this);
		}
		else
		{
			getGUI()->setCurrentTab(i.value());
		}
	}

	void SyndicationPlugin::downloadLink(const KUrl & url)
	{
		LinkDownloader* dlr = new LinkDownloader(url,getCore(),true);
		dlr->start();
	}
	
	void SyndicationPlugin::updateTabText(QWidget* w,const QString & text)
	{
		getGUI()->setTabText(w,text);
	}
	
	void SyndicationPlugin::showFeed()
	{
		QModelIndexList idx = feed_view->selectedFeeds();
		foreach (const QModelIndex & i,idx)
		{
			Feed* f = feed_list->feedForIndex(i);
			if (f && !tabs.contains(f))
			{
				activateFeedWidget(f);
			}
		}
	}
}
