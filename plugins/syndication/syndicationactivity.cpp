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
#include <QHBoxLayout>
#include <QToolButton>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kmainwindow.h>
#include <interfaces/functions.h>
#include <interfaces/guiinterface.h>
#include <util/error.h>
#include <util/fileops.h>
#include <syndication/loader.h>
#include "syndicationactivity.h"
#include "feedwidget.h"
#include "feed.h"
#include "feedlist.h"
#include "feedlistview.h"
#include "filter.h"
#include "filterlist.h"
#include "filterlistview.h"
#include "filtereditor.h"
#include "managefiltersdlg.h"
#include "syndicationtab.h"
#include "syndicationplugin.h"
#include "linkdownloader.h"

namespace kt
{
	SyndicationActivity::SyndicationActivity(SyndicationPlugin* sp,QWidget* parent) 
		: Activity(i18n("Syndication"),"application-rss+xml",parent),sp(sp)
	{
		QString ddir = kt::DataDir() + "syndication/";
		if (!bt::Exists(ddir))
			bt::MakeDir(ddir,true);
		
		setToolTip(i18n("Manages RSS and Atom feeds"));
		QHBoxLayout* layout = new QHBoxLayout(this);
		splitter = new QSplitter(Qt::Horizontal,this);
		layout->addWidget(splitter);
		
		feed_list = new FeedList(ddir,this);
		filter_list = new FilterList(this);
		tab = new SyndicationTab(sp->actionCollection(),feed_list,filter_list,splitter);
		splitter->addWidget(tab);
		
		tabs = new KTabWidget(splitter);
		splitter->addWidget(tabs);
		splitter->setStretchFactor(0,1);
		splitter->setStretchFactor(1,3);
		
		connect(tab->feedView(),SIGNAL(feedActivated(Feed*)),this,SLOT(activateFeedWidget(Feed*)));
		connect(tab->feedView(),SIGNAL(enableRemove(bool)),sp->remove_feed,SLOT(setEnabled(bool)));
		connect(tab->feedView(),SIGNAL(enableRemove(bool)),sp->show_feed,SLOT(setEnabled(bool)));
		connect(tab->feedView(),SIGNAL(enableRemove(bool)),sp->manage_filters,SLOT(setEnabled(bool)));
		connect(tab->filterView(),SIGNAL(filterActivated(Filter*)),this,SLOT(editFilter(Filter*)));
		connect(tab->filterView(),SIGNAL(enableRemove(bool)),sp->remove_filter,SLOT(setEnabled(bool)));
		connect(tab->filterView(),SIGNAL(enableEdit(bool)),sp->edit_filter,SLOT(setEnabled(bool)));
	
		tabs->hide();
		filter_list->loadFilters(kt::DataDir() + "syndication/filters");
		feed_list->loadFeeds(filter_list,this);
		feed_list->importOldFeeds();
		
		QToolButton* rc = new QToolButton(tabs);
		tabs->setCornerWidget(rc,Qt::TopRightCorner);
		rc->setIcon(KIcon("tab-close"));
		connect(rc,SIGNAL(clicked()),this,SLOT(closeTab()));
	}

	SyndicationActivity::~SyndicationActivity() 
	{
	}

	void SyndicationActivity::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("SyndicationActivity");
		QStringList tab_list;
		tab_list = g.readEntry("tabs",tab_list);
		foreach (const QString & tab,tab_list)
		{
			Feed* f = feed_list->feedForDirectory(tab);
			if (f)
				activateFeedWidget(f);
		}
		
		tabs->setCurrentIndex(g.readEntry("current_tab",0));
		QByteArray state = g.readEntry("splitter",QByteArray());
		splitter->restoreState(state);
		tab->loadState(g);
	}

	void SyndicationActivity::saveState(KSharedConfigPtr cfg)
	{
		QStringList active_tabs;
		int cnt = tabs->count();
		for (int i = 0;i < cnt;i++)
		{
			FeedWidget* fw = (FeedWidget*)tabs->widget(i);
			active_tabs << fw->getFeed()->directory();
		}
		
		KConfigGroup g = cfg->group("SyndicationActivity");
		g.writeEntry("tabs",active_tabs);
		g.writeEntry("current_tab",tabs->currentIndex());
		g.writeEntry("splitter",splitter->saveState());
		tab->saveState(g);
		g.sync();
	}
	
	void SyndicationActivity::addFeed()
	{
		bool ok = false;
		QString url = KInputDialog::getText(i18n("Enter the URL"),i18n("Please enter the URL of the RSS or Atom feed."),
					QString(),&ok,sp->getGUI()->getMainWindow());
		if (!ok || url.isEmpty())
			return;
					
		Syndication::Loader *loader = Syndication::Loader::create(this,SLOT(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
		loader->loadFrom(KUrl(url));
		downloads.insert(loader,KUrl(url));
	}

	void SyndicationActivity::loadingComplete(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode status)
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
	
	FeedWidget* SyndicationActivity::feedWidget(Feed* f)
	{
		int cnt = tabs->count();
		for (int i = 0;i < cnt;i++)
		{
			FeedWidget* fw = (FeedWidget*)tabs->widget(i);
			if (fw->getFeed() == f)
				return fw;
		}
		
		return 0;
	}

	void SyndicationActivity::removeFeed()
	{
		QModelIndexList idx = tab->feedView()->selectedFeeds();
		foreach (const QModelIndex & i,idx)
		{
			Feed* f = feed_list->feedForIndex(i);
			// close tab window if one is open for this feed
			if (f)
			{
				FeedWidget* fw = feedWidget(f);
				if (fw)
				{
					tabs->removePage(fw);
					delete fw;
				}
			}
		}
		feed_list->removeFeeds(idx);
	}
	
	void SyndicationActivity::activateFeedWidget(Feed* f)
	{
		if (!f)
			return;
		
		FeedWidget* fw = feedWidget(f);
		if (!fw)
		{
			fw = new FeedWidget(f,filter_list,this,tabs);
			connect(fw,SIGNAL(updateCaption(QWidget*, const QString&)),this,SLOT(updateTabText(QWidget*, const QString&)));
			tabs->addTab(fw,KIcon("application-rss+xml"),f->title());
			if (tabs->count() == 1)
			{
				tabs->show();
			}
			tabs->setCurrentWidget(fw);
		}
		else
		{
			tabs->setCurrentWidget(fw);
		}
	}
	
	void SyndicationActivity::closeTab()
	{
		int idx = tabs->currentIndex();
		if (idx >= 0)
		{
			QWidget* w = tabs->widget(idx);
			tabs->removeTab(idx);
			delete w;
			if (tabs->count() == 0)
			{
				tabs->hide();
			}
		}
	}
	
	void SyndicationActivity::downloadLink(const KUrl & url,const QString & group,const QString & location,bool silently)
	{
		LinkDownloader* dlr = new LinkDownloader(url,sp->getCore(),!silently,group,location);
		dlr->start();
	}
	
	void SyndicationActivity::updateTabText(QWidget* w,const QString & text)
	{
		int idx = tabs->indexOf(w);
		if (idx >= 0)
			tabs->setTabText(idx,text);
	}
	
	void SyndicationActivity::showFeed()
	{
		QModelIndexList idx = tab->feedView()->selectedFeeds();
		foreach (const QModelIndex & i,idx)
		{
			Feed* f = feed_list->feedForIndex(i);
			if (f)
			{
				activateFeedWidget(f);
			}
		}
	}
	
	Filter* SyndicationActivity::addNewFilter()
	{
		Filter* filter = new Filter(i18n("New Filter"));
		FilterEditor dlg(filter,filter_list,feed_list,sp->getCore(),sp->getGUI()->getMainWindow());
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
	
	void SyndicationActivity::addFilter()
	{
		addNewFilter();
	}
	
	void SyndicationActivity::removeFilter()
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
	
	void SyndicationActivity::editFilter()
	{
		QModelIndexList idx = tab->filterView()->selectedFilters();
		if (idx.count() == 0)
			return;
		
		Filter* f = filter_list->filterForIndex(idx.front());
		if (f)
			editFilter(f);
	}
	
	void SyndicationActivity::editFilter(Filter* f)
	{
		FilterEditor dlg(f,filter_list,feed_list,sp->getCore(),sp->getGUI()->getMainWindow());
		if (dlg.exec() == QDialog::Accepted)
		{
			filter_list->filterEdited(f);
			filter_list->saveFilters(kt::DataDir() + "syndication/filters");
			feed_list->filterEdited(f);
		}
	}
	
	void SyndicationActivity::manageFilters()
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