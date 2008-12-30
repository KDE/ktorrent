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
#include "syndicationtab.h"
#include "feed.h"
#include "feedlist.h"
#include "feedlistview.h"
#include "feedwidget.h"
#include "linkdownloader.h"
#include "filterlist.h"
#include "filter.h"
#include "filterlistview.h"
#include "filtereditor.h"
#include "managefiltersdlg.h"

K_EXPORT_COMPONENT_FACTORY(ktsyndicationplugin,KGenericFactory<kt::SyndicationPlugin>("ktsyndicationplugin"))
		
using namespace bt;

namespace kt
{

	SyndicationPlugin::SyndicationPlugin(QObject* parent,const QStringList& args): Plugin(parent),add_feed(0)
	{
		Q_UNUSED(args);
		tab = 0;
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
		filter_list = new FilterList(this);
		tab = new SyndicationTab(actionCollection(),feed_list,filter_list,0);
		connect(tab->feedView(),SIGNAL(feedActivated(Feed*)),this,SLOT(activateFeedWidget(Feed*)));
		connect(tab->feedView(),SIGNAL(enableRemove(bool)),remove_feed,SLOT(setEnabled(bool)));
		connect(tab->feedView(),SIGNAL(enableRemove(bool)),show_feed,SLOT(setEnabled(bool)));
		connect(tab->feedView(),SIGNAL(enableRemove(bool)),manage_filters,SLOT(setEnabled(bool)));
		connect(tab->filterView(),SIGNAL(filterActivated(Filter*)),this,SLOT(editFilter(Filter*)));
		connect(tab->filterView(),SIGNAL(enableRemove(bool)),remove_filter,SLOT(setEnabled(bool)));
		connect(tab->filterView(),SIGNAL(enableEdit(bool)),edit_filter,SLOT(setEnabled(bool)));
		getGUI()->addToolWidget(tab,"application-rss+xml",i18n("Syndication"),
			   i18n("Widget to manage RSS and Atom feeds"),GUIInterface::DOCK_LEFT);
		filter_list->loadFilters(kt::DataDir() + "syndication/filters");
		feed_list->loadFeeds(filter_list,this);
		feed_list->importOldFeeds();
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
		
		getGUI()->removeToolWidget(tab);
		delete tab;
		delete feed_list;
		delete filter_list;
		tab = 0;
		feed_list = 0;
		filter_list = 0;
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
			KMessageBox::error(tab,i18n("Failed to load feed %1 !",downloads[loader].prettyUrl()));
			downloads.remove(loader);
			return;
		}
		
		try
		{
			QString ddir = kt::DataDir() + "syndication/";
			Feed* f = new Feed(downloads[loader],feed,Feed::newFeedDir(ddir));
			connect(f,SIGNAL(downloadLink(const KUrl&, const QString&, const QString&, bool)),
					this,SLOT(downloadLink(const KUrl&, const QString&, const QString&, bool)));
			f->save();
			feed_list->addFeed(f);
		}
		catch (bt::Error & err)
		{
			KMessageBox::error(tab,i18n("Failed to create directory for feed %1 : %2",downloads[loader].prettyUrl(),err.toString()));
		}
		downloads.remove(loader);
	}
	
	void SyndicationPlugin::removeFeed()
	{
		QModelIndexList idx = tab->feedView()->selectedFeeds();
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
		
		add_feed = new KAction(KIcon("kt-add-feeds"),i18n("Add Feed"),this);
		connect(add_feed,SIGNAL(triggered()),this,SLOT(addFeed()));
		ac->addAction("add_feed",add_feed);
		
		remove_feed = new KAction(KIcon("kt-remove-feeds"),i18n("Remove Feed"),this);
		connect(remove_feed,SIGNAL(triggered()),this,SLOT(removeFeed()));
		ac->addAction("remove_feed",remove_feed);
		
		show_feed = new KAction(KIcon("tab-new"),i18n("Show Feed"),this);
		connect(show_feed,SIGNAL(triggered()),this,SLOT(showFeed()));
		ac->addAction("show_feed",show_feed);
		
		manage_filters = new KAction(KIcon("view-filter"),i18n("Add/Remove Filters"),this);
		connect(manage_filters,SIGNAL(triggered()),this,SLOT(manageFilters()));
		ac->addAction("manage_filters",manage_filters);
		
		add_filter = new KAction(KIcon("kt-add-filters"),i18n("Add Filter"),this);
		connect(add_filter,SIGNAL(triggered()),this,SLOT(addFilter()));
		ac->addAction("add_filter",add_filter);
		
		remove_filter = new KAction(KIcon("kt-remove-filters"),i18n("Remove Filter"),this);
		connect(remove_filter,SIGNAL(triggered()),this,SLOT(removeFilter()));
		ac->addAction("remove_filter",remove_filter);
		
		edit_filter = new KAction(KIcon("preferences-other"),i18n("Edit Filter"),this);
		connect(edit_filter,SIGNAL(triggered()),this,SLOT(editFilter()));
		ac->addAction("edit_filter",edit_filter);
		
		remove_filter->setEnabled(false);
		edit_filter->setEnabled(false);
		remove_feed->setEnabled(false);
		show_feed->setEnabled(false);
		manage_filters->setEnabled(false);
	}
	
	void SyndicationPlugin::tabCloseRequest(kt::GUIInterface* gui, QWidget* tab)
	{
		QMap<Feed*,FeedWidget*>::iterator i = tabs.begin();
		while (i != tabs.end())
		{
			if (i.value() == tab)
			{
				gui->removeTabPage(i.value());
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
			FeedWidget* fw = new FeedWidget(f,filter_list,this,0);
			connect(fw,SIGNAL(updateCaption(QWidget*, const QString&)),this,SLOT(updateTabText(QWidget*, const QString&)));
			tabs.insert(f,fw);
			getGUI()->addTabPage(fw,"application-rss+xml",f->title(),i18n("Feed %1",f->feedUrl().prettyUrl()),this);
		}
		else
		{
			getGUI()->setCurrentTab(i.value());
		}
	}

	void SyndicationPlugin::downloadLink(const KUrl & url,const QString & group,const QString & location,bool silently)
	{
		LinkDownloader* dlr = new LinkDownloader(url,getCore(),!silently,group,location);
		dlr->start();
	}
	
	void SyndicationPlugin::updateTabText(QWidget* w,const QString & text)
	{
		getGUI()->setTabText(w,text);
	}
	
	void SyndicationPlugin::showFeed()
	{
		QModelIndexList idx = tab->feedView()->selectedFeeds();
		foreach (const QModelIndex & i,idx)
		{
			Feed* f = feed_list->feedForIndex(i);
			if (f && !tabs.contains(f))
			{
				activateFeedWidget(f);
			}
		}
	}
	
	Filter* SyndicationPlugin::addNewFilter()
	{
		Filter* filter = new Filter(i18n("New Filter"));
		FilterEditor dlg(filter,filter_list,feed_list,getCore(),getGUI()->getMainWindow());
		dlg.setWindowTitle(i18n("Add New Filter"));
		if (dlg.exec() == QDialog::Accepted)
		{
			filter_list->addFilter(filter);
			filter_list->saveFilters(kt::DataDir() + "syndication/filters");
			return filter;
		}
		else
		{
			delete filter;
			return 0;
		}
	}
	
	void SyndicationPlugin::addFilter()
	{
		addNewFilter();
	}
	
	void SyndicationPlugin::removeFilter()
	{
		QModelIndexList indexes = tab->filterView()->selectedFilters();
		QList<Filter*> to_remove;
		foreach (const QModelIndex & idx,indexes)
		{
			Filter* f = filter_list->filterForIndex(idx);
			if (f)
				to_remove.append(f);
		}
		
		foreach (Filter* f,to_remove)
		{
			feed_list->filterRemoved(f);
			filter_list->removeFilter(f);
			delete f;
		}
		
		filter_list->saveFilters(kt::DataDir() + "syndication/filters");
	}
	
	void SyndicationPlugin::editFilter()
	{
		QModelIndexList idx = tab->filterView()->selectedFilters();
		if (idx.count() == 0)
			return;
		
		Filter* f = filter_list->filterForIndex(idx.front());
		if (f)
			editFilter(f);
	}
	
	void SyndicationPlugin::editFilter(Filter* f)
	{
		FilterEditor dlg(f,filter_list,feed_list,getCore(),getGUI()->getMainWindow());
		if (dlg.exec() == QDialog::Accepted)
		{
			filter_list->filterEdited(f);
			filter_list->saveFilters(kt::DataDir() + "syndication/filters");
			feed_list->filterEdited(f);
		}
	}
	
	void SyndicationPlugin::manageFilters()
	{
		QModelIndexList idx = tab->feedView()->selectedFeeds();
		if (idx.count() == 0)
			return;
		
		Feed* f = feed_list->feedForIndex(idx.front());
		if (!f)
			return;
		
		ManageFiltersDlg dlg(f,filter_list,this,tab);
		if (dlg.exec() == QDialog::Accepted)
		{
			f->save();
			f->runFilters();
		}
	}
}
