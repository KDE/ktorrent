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
#include "rssfeedmanager.h"

#include <kdirlister.h>
#include <kfileitem.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <keditlistbox.h>
// #include <kmimetype.h>
#include <kmessagebox.h>

#include <qstring.h>
#include <qobject.h>
#include <qfile.h>
#include <qdatetime.h>

#include <qlineedit.h>
#include <kurlrequester.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qdatetimeedit.h>
#include <qtable.h>
#include <qregexp.h>
#include <qlayout.h>

#include <torrent/globals.h>
#include <util/log.h>
#include <util/constants.h>

#include <interfaces/coreinterface.h>

#include <qapplication.h>

#include "../../libktorrent/torrent/bdecoder.h"
#include "../../libktorrent/torrent/bnode.h"

#include "rsslinkdownloader.h"

using namespace bt;

namespace kt
{

	RssFeedManager::RssFeedManager(CoreInterface* core, QWidget * parent) : RssFeedWidget(parent)
	{
		//Construct the manager
		m_core = core;
		currentFeed = -1;
		currentAcceptFilter = -1;
		currentRejectFilter = -1;
		
		feedListSaving = false;
		filterListSaving = false;
		
		//get the articles list setup
		feedArticles->setLeftMargin(0);
		feedArticles->verticalHeader()->hide();
		feedArticles->setNumCols(3);
		feedArticles->setColumnLabels(QStringList() << i18n("Title") << i18n("Description") << i18n("Link"));
		feedArticles->horizontalHeader()->setStretchEnabled(true, 0);
		feedArticles->hideColumn(1);
		feedArticles->hideColumn(2);
		
		//get the matches list setup
		filterMatches->setLeftMargin(0);
		filterMatches->verticalHeader()->hide();
		filterMatches->setNumCols(4);
		filterMatches->setColumnLabels(QStringList() << i18n("Season") << i18n("Episode") << i18n("Time") << i18n("Link"));
		filterMatches->setColumnWidth(0, 60);
		filterMatches->setColumnWidth(1, 60);
		filterMatches->setColumnWidth(2, 180);
		filterMatches->horizontalHeader()->setStretchEnabled(true, 3);
		
		loadFeedList();
		loadFilterList();
		
		//connect the buttons
		connect(newFeed, SIGNAL(clicked()), this, SLOT(addNewFeed() ) );
		connect(deleteFeed, SIGNAL(clicked()), this, SLOT(deleteSelectedFeed() ) );
		
		connect(newAcceptFilter, SIGNAL(clicked()), this, SLOT(addNewAcceptFilter() ) );
		connect(deleteAcceptFilter, SIGNAL(clicked()), this, SLOT(deleteSelectedAcceptFilter() ) );
		
		connect(newRejectFilter, SIGNAL(clicked()), this, SLOT(addNewRejectFilter() ) );
		connect(deleteRejectFilter, SIGNAL(clicked()), this, SLOT(deleteSelectedRejectFilter() ) );
		
		//connect the changing of the active feed
		connect(feedlist, SIGNAL(selectionChanged()), this, SLOT(changedActiveFeed()) );
		
		//connect the changing of the url to enable the refresh button
		connect(feedUrl, SIGNAL(textChanged(const QString &)), this, SLOT(changedFeedUrl()) );
		
		//connect the changing of the filters
		connect(acceptFilterList, SIGNAL(selectionChanged()), this, SLOT(changedActiveAcceptFilter()) );
		connect(rejectFilterList, SIGNAL(selectionChanged()), this, SLOT(changedActiveRejectFilter()) );
		
		//connect the selection and downloading of articles
		connect(feedArticles, SIGNAL(selectionChanged()), this, SLOT(changedArticleSelection()) );
		connect(downloadArticle, SIGNAL(clicked()), this, SLOT(downloadSelectedArticles()) );
		
		//connect the selection, downloading and deletion of matches
		connect(filterMatches, SIGNAL(selectionChanged()), this, SLOT(changedMatchSelection()) );
		connect(downloadFilterMatch, SIGNAL(clicked()), this, SLOT(downloadSelectedMatches()) );
		connect(deleteFilterMatch, SIGNAL(clicked()), this, SLOT(deleteSelectedMatches()) );
		
		//connect the test text update to the slot
		connect(testText, SIGNAL(textChanged(const QString &)), this, SLOT(testTextChanged()) );
		connect(testTestText, SIGNAL(clicked()), this, SLOT(testFilter()) );
		
		changedActiveFeed();
		changedActiveAcceptFilter();
		
	}

	RssFeedManager::~RssFeedManager()
	{
		//Destruct the manager
	}
	
	void RssFeedManager::clearArticles()
	{
		int pos = feeds.find((RssFeed *)sender());
			
		if (pos >= 0)
		{
			feeds.at(pos)->clearArticles();
			if (feedlist->isSelected(pos))
			{
				//this feed is active so we should update the display
				feedArticles->setNumRows(0);
			}
		}
	}
	
	void RssFeedManager::changedFeedUrl()
	{
		refreshFeed->setEnabled(!feedUrl->url().isEmpty());
	}
	
	void RssFeedManager::connectFeed(int index)
	{
	
		connect(feedTitle, SIGNAL(textChanged(const QString &)), feeds.at(index), SLOT(setTitle(const QString &) ) );
		connect(feeds.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(setFeedTitle(const QString &) ) );
		
		//url
		connect(feedUrl, SIGNAL(textChanged(const QString &)), feeds.at(index), SLOT(setFeedUrl(const QString&) ) );
		connect(feeds.at(index), SIGNAL(feedUrlChanged(const KURL&)), feedUrl, SLOT(setKURL(const KURL&) ) );
		
		//articleAge
		connect(feedArticleAge, SIGNAL(valueChanged(int)), feeds.at(index), SLOT(setArticleAge(int) ) );
		connect(feeds.at(index), SIGNAL(articleAgeChanged(int)), feedArticleAge, SLOT(setValue(int) ) );
		
		//active
		connect(feedActive, SIGNAL(toggled(bool)), feeds.at(index), SLOT(setActive(bool) ) );
		connect(feeds.at(index), SIGNAL(activeChanged(bool)), feedActive, SLOT(setChecked(bool) ) );
		
		//autoRefresh
		connect(feedAutoRefresh, SIGNAL(valueChanged(const QTime&)), feeds.at(index), SLOT(setAutoRefresh(const QTime&) ) );
		connect(feeds.at(index), SIGNAL(autoRefreshChanged(const QTime&)), feedAutoRefresh, SLOT(setTime(const QTime&) ) );
	
		//ignoreTTL
		connect(feedIgnoreTTL, SIGNAL(toggled(bool)), feeds.at(index), SLOT(setIgnoreTTL(bool) ) );
		connect(feeds.at(index), SIGNAL(ignoreTTLChanged(bool)), feedIgnoreTTL, SLOT(setChecked(bool) ) );
		
		//articles
		connect(feeds.at(index), SIGNAL(articlesChanged(const RssArticle::List&)), this, SLOT(updateArticles(const RssArticle::List&) ) );
		
		//connect the refresh button
		connect(refreshFeed, SIGNAL(clicked()), feeds.at(index), SLOT(refreshFeed()) );
	}
	
	void RssFeedManager::disconnectFeed(int index)
	{
		disconnect(feedTitle, SIGNAL(textChanged(const QString &)), feeds.at(index), SLOT(setTitle(const QString &) ) );
		disconnect(feeds.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(setFeedTitle(const QString &) ) );
		
		//url
		disconnect(feedUrl, SIGNAL(textChanged(const QString &)), feeds.at(index), SLOT(setFeedUrl(const QString&) ) );
		disconnect(feeds.at(index), SIGNAL(feedUrlChanged(const KURL&)), feedUrl, SLOT(setKURL(const KURL&) ) );
		
		//articleAge
		disconnect(feedArticleAge, SIGNAL(valueChanged(int)), feeds.at(index), SLOT(setArticleAge(int) ) );
		disconnect(feeds.at(index), SIGNAL(articleAgeChanged(int)), feedArticleAge, SLOT(setValue(int) ) );
		
		//active
		disconnect(feedActive, SIGNAL(toggled(bool)), feeds.at(index), SLOT(setActive(bool) ) );
		disconnect(feeds.at(index), SIGNAL(activeChanged(bool)), feedActive, SLOT(setChecked(bool) ) );
		
		//autoRefresh
		disconnect(feedAutoRefresh, SIGNAL(valueChanged(const QTime&)), feeds.at(index), SLOT(setAutoRefresh(const QTime&) ) );
		disconnect(feeds.at(index), SIGNAL(autoRefreshChanged(const QTime&)), feedAutoRefresh, SLOT(setTime(const QTime&) ) );
	
		//ignoreTTL
		disconnect(feedIgnoreTTL, SIGNAL(toggled(bool)), feeds.at(index), SLOT(setIgnoreTTL(bool) ) );
		disconnect(feeds.at(index), SIGNAL(ignoreTTLChanged(bool)), feedIgnoreTTL, SLOT(setChecked(bool) ) );
		
		//articles
		disconnect(feeds.at(index), SIGNAL(articlesChanged(const RssArticle::List&)), this, SLOT(updateArticles(const RssArticle::List&) ) );
		
		disconnect(refreshFeed, SIGNAL(clicked()), feeds.at(index), SLOT(refreshFeed()) );
	}
	
	void RssFeedManager::connectFilter(int index, bool acceptFilter)
	{
		if (acceptFilter)
		{
		//title
		connect(filterTitle, SIGNAL(textChanged(const QString &)), acceptFilters.at(index), SLOT(setTitle(const QString &) ) );
		connect(acceptFilters.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(setFilterTitle(const QString &) ) );
		//active
		connect(filterActive, SIGNAL(toggled(bool)), acceptFilters.at(index), SLOT(setActive(bool) ) );
		connect(acceptFilters.at(index), SIGNAL(activeChanged(bool)), filterActive, SLOT(setChecked(bool) ) );
		//regExps
		connect(filterRegExps, SIGNAL(changed()), this, SLOT(updateRegExps()) );
		//series
		connect(filterSeries, SIGNAL(toggled(bool)), acceptFilters.at(index), SLOT(setSeries(bool) ) );
		connect(acceptFilters.at(index), SIGNAL(seriesChanged(bool)), filterSeries, SLOT(setChecked(bool) ) );
		//sansEpisode
		connect(filterSansEpisode, SIGNAL(toggled(bool)), acceptFilters.at(index), SLOT(setSansEpisode(bool) ) );
		connect(acceptFilters.at(index), SIGNAL(sansEpisodeChanged(bool)), filterSansEpisode, SLOT(setChecked(bool) ) );
		//minSeason
		connect(filterMinSeason, SIGNAL(valueChanged(int)), acceptFilters.at(index), SLOT(setMinSeason(int) ) );
		connect(acceptFilters.at(index), SIGNAL(minSeasonChanged(int)), filterMinSeason, SLOT(setValue(int) ) );
		//minEpisode
		connect(filterMinEpisode, SIGNAL(valueChanged(int)), acceptFilters.at(index), SLOT(setMinEpisode(int) ) );
		connect(acceptFilters.at(index), SIGNAL(minEpisodeChanged(int)), filterMinEpisode, SLOT(setValue(int) ) );
		//maxSeason
		connect(filterMaxSeason, SIGNAL(valueChanged(int)), acceptFilters.at(index), SLOT(setMaxSeason(int) ) );
		connect(acceptFilters.at(index), SIGNAL(maxSeasonChanged(int)), filterMaxSeason, SLOT(setValue(int) ) );
		//maxEpisode
		connect(filterMaxEpisode, SIGNAL(valueChanged(int)), acceptFilters.at(index), SLOT(setMaxEpisode(int) ) );
		connect(acceptFilters.at(index), SIGNAL(maxEpisodeChanged(int)), filterMaxEpisode, SLOT(setValue(int) ) );
		//matches
		connect(acceptFilters.at(index), SIGNAL(matchesChanged(const QValueList<FilterMatch>&)), this, SLOT(updateMatches(const QValueList<FilterMatch>&) ) );
		
		connect(processFilter, SIGNAL(clicked()), acceptFilters.at(index), SIGNAL(rescanFilter()) );

		}
		else
		{
		//title
		connect(filterTitle, SIGNAL(textChanged(const QString &)), rejectFilters.at(index), SLOT(setTitle(const QString &) ) );
		connect(rejectFilters.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(setFilterTitle(const QString &) ) );
		//active
		connect(filterActive, SIGNAL(toggled(bool)), rejectFilters.at(index), SLOT(setActive(bool) ) );
		connect(rejectFilters.at(index), SIGNAL(activeChanged(bool)), filterActive, SLOT(setChecked(bool) ) );
		//regExps
		connect(filterRegExps, SIGNAL(changed()), this, SLOT(updateRegExps()) );
		//series
		connect(filterSeries, SIGNAL(toggled(bool)), rejectFilters.at(index), SLOT(setSeries(bool) ) );
		connect(rejectFilters.at(index), SIGNAL(seriesChanged(bool)), filterSeries, SLOT(setChecked(bool) ) );
		//sansEpisode
		connect(filterSansEpisode, SIGNAL(toggled(bool)), rejectFilters.at(index), SLOT(setSansEpisode(bool) ) );
		connect(rejectFilters.at(index), SIGNAL(sansEpisodeChanged(bool)), filterSansEpisode, SLOT(setChecked(bool) ) );
		//minSeason
		connect(filterMinSeason, SIGNAL(valueChanged(int)), rejectFilters.at(index), SLOT(setMinSeason(int) ) );
		connect(rejectFilters.at(index), SIGNAL(minSeasonChanged(int)), filterMinSeason, SLOT(setValue(int) ) );
		//minEpisode
		connect(filterMinEpisode, SIGNAL(valueChanged(int)), rejectFilters.at(index), SLOT(setMinEpisode(int) ) );
		connect(rejectFilters.at(index), SIGNAL(minEpisodeChanged(int)), filterMinEpisode, SLOT(setValue(int) ) );
		//maxSeason
		connect(filterMaxSeason, SIGNAL(valueChanged(int)), rejectFilters.at(index), SLOT(setMaxSeason(int) ) );
		connect(rejectFilters.at(index), SIGNAL(maxSeasonChanged(int)), filterMaxSeason, SLOT(setValue(int) ) );
		//maxEpisode
		connect(filterMaxEpisode, SIGNAL(valueChanged(int)), rejectFilters.at(index), SLOT(setMaxEpisode(int) ) );
		connect(rejectFilters.at(index), SIGNAL(maxEpisodeChanged(int)), filterMaxEpisode, SLOT(setValue(int) ) );
		//matches
		connect(rejectFilters.at(index), SIGNAL(matchesChanged(const QValueList<FilterMatch>&)), this, SLOT(updateMatches(const QValueList<FilterMatch>&) ) );
		
		connect(processFilter, SIGNAL(clicked()), rejectFilters.at(index), SIGNAL(rescanFilter()) );
		
		}
	}
	
	void RssFeedManager::disconnectFilter(int index, bool acceptFilter)
	{
		if (acceptFilter)
		{
		//title
		disconnect(filterTitle, SIGNAL(textChanged(const QString &)), acceptFilters.at(index), SLOT(setTitle(const QString &) ) );
		disconnect(acceptFilters.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(setFilterTitle(const QString &) ) );
		//active
		disconnect(filterActive, SIGNAL(toggled(bool)), acceptFilters.at(index), SLOT(setActive(bool) ) );
		disconnect(acceptFilters.at(index), SIGNAL(activeChanged(bool)), filterActive, SLOT(setChecked(bool) ) );
		//regExps
		disconnect(filterRegExps, SIGNAL(changed()), this, SLOT(updateRegExps()) );
		//series
		disconnect(filterSeries, SIGNAL(toggled(bool)), acceptFilters.at(index), SLOT(setSeries(bool) ) );
		disconnect(acceptFilters.at(index), SIGNAL(seriesChanged(bool)), filterSeries, SLOT(setChecked(bool) ) );
		//sansEpisode
		disconnect(filterSansEpisode, SIGNAL(toggled(bool)), acceptFilters.at(index), SLOT(setSansEpisode(bool) ) );
		disconnect(acceptFilters.at(index), SIGNAL(sansEpisodeChanged(bool)), filterSansEpisode, SLOT(setChecked(bool) ) );
		//minSeason
		disconnect(filterMinSeason, SIGNAL(valueChanged(int)), acceptFilters.at(index), SLOT(setMinSeason(int) ) );
		disconnect(acceptFilters.at(index), SIGNAL(minSeasonChanged(int)), filterMinSeason, SLOT(setValue(int) ) );
		//minEpisode
		disconnect(filterMinEpisode, SIGNAL(valueChanged(int)), acceptFilters.at(index), SLOT(setMinEpisode(int) ) );
		disconnect(acceptFilters.at(index), SIGNAL(minEpisodeChanged(int)), filterMinEpisode, SLOT(setValue(int) ) );
		//maxSeason
		disconnect(filterMaxSeason, SIGNAL(valueChanged(int)), acceptFilters.at(index), SLOT(setMaxSeason(int) ) );
		disconnect(acceptFilters.at(index), SIGNAL(maxSeasonChanged(int)), filterMaxSeason, SLOT(setValue(int) ) );
		//maxEpisode
		disconnect(filterMaxEpisode, SIGNAL(valueChanged(int)), acceptFilters.at(index), SLOT(setMaxEpisode(int) ) );
		disconnect(acceptFilters.at(index), SIGNAL(maxEpisodeChanged(int)), filterMaxEpisode, SLOT(setValue(int) ) );
		//matches
		disconnect(acceptFilters.at(index), SIGNAL(matchesChanged(const QValueList<FilterMatch>&)), this, SLOT(updateMatches(const QValueList<FilterMatch>&) ) ); 
		
		disconnect(processFilter, SIGNAL(clicked()), acceptFilters.at(index), SIGNAL(rescanFilter()) );
		}
		else
		{
		//title
		disconnect(filterTitle, SIGNAL(textChanged(const QString &)), rejectFilters.at(index), SLOT(setTitle(const QString &) ) );
		disconnect(rejectFilters.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(setFilterTitle(const QString &) ) );
		//active
		disconnect(filterActive, SIGNAL(toggled(bool)), rejectFilters.at(index), SLOT(setActive(bool) ) );
		disconnect(rejectFilters.at(index), SIGNAL(activeChanged(bool)), filterActive, SLOT(setChecked(bool) ) );
		//regExps
		disconnect(filterRegExps, SIGNAL(changed()), this, SLOT(updateRegExps()) );
		//series
		disconnect(filterSeries, SIGNAL(toggled(bool)), rejectFilters.at(index), SLOT(setSeries(bool) ) );
		disconnect(rejectFilters.at(index), SIGNAL(seriesChanged(bool)), filterSeries, SLOT(setChecked(bool) ) );
		//sansEpisode
		disconnect(filterSansEpisode, SIGNAL(toggled(bool)), rejectFilters.at(index), SLOT(setSansEpisode(bool) ) );
		disconnect(rejectFilters.at(index), SIGNAL(sansEpisodeChanged(bool)), filterSansEpisode, SLOT(setChecked(bool) ) );
		//minSeason
		disconnect(filterMinSeason, SIGNAL(valueChanged(int)), rejectFilters.at(index), SLOT(setMinSeason(int) ) );
		disconnect(rejectFilters.at(index), SIGNAL(minSeasonChanged(int)), filterMinSeason, SLOT(setValue(int) ) );
		//minEpisode
		disconnect(filterMinEpisode, SIGNAL(valueChanged(int)), rejectFilters.at(index), SLOT(setMinEpisode(int) ) );
		disconnect(rejectFilters.at(index), SIGNAL(minEpisodeChanged(int)), filterMinEpisode, SLOT(setValue(int) ) );
		//maxSeason
		disconnect(filterMaxSeason, SIGNAL(valueChanged(int)), rejectFilters.at(index), SLOT(setMaxSeason(int) ) );
		disconnect(rejectFilters.at(index), SIGNAL(maxSeasonChanged(int)), filterMaxSeason, SLOT(setValue(int) ) );
		//maxEpisode
		disconnect(filterMaxEpisode, SIGNAL(valueChanged(int)), rejectFilters.at(index), SLOT(setMaxEpisode(int) ) );
		disconnect(rejectFilters.at(index), SIGNAL(maxEpisodeChanged(int)), filterMaxEpisode, SLOT(setValue(int) ) );
		//matches
		disconnect(rejectFilters.at(index), SIGNAL(matchesChanged(const QValueList<FilterMatch>&)), this, SLOT(updateMatches(const QValueList<FilterMatch>&) ) );
		
		disconnect(processFilter, SIGNAL(clicked()), rejectFilters.at(index), SIGNAL(rescanFilter()) );
		
		}
	}
	
	void RssFeedManager::addNewFeed(RssFeed feed)
	{
		if (feeds.isEmpty())
			deleteFeed->setEnabled(true);
		
		feeds.append(new RssFeed(feed));
		
		int index = feeds.count()-1;
		feedlist->insertItem(feeds.at(index)->title());
		feedlist->setCurrentItem(index);
		
		//update the feed list
		connect(feeds.at(index), SIGNAL(titleChanged(const QString&)), this, SLOT(updateFeedList()) );
		
		//clear the articles list when the url is changed
		connect(feeds.at(index), SIGNAL(feedUrlChanged(const KURL&)), this, SLOT(clearArticles() ) );
		
		//connect the scanArticle signal to the scanArticle slot
		connect(feeds.at(index), SIGNAL(scanRssArticle(RssArticle)), this, SLOT(scanArticle(RssArticle) ) );
		
		//connect all the fields to the save slot
		//title
		connect(feeds.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(saveFeedList() ) );
		//url
		connect(feeds.at(index), SIGNAL(feedUrlChanged(const KURL&)), this, SLOT(saveFeedList() ) );
		//articleAge
		connect(feeds.at(index), SIGNAL(articleAgeChanged(int)), this, SLOT(saveFeedList() ) );
		//active
		connect(feeds.at(index), SIGNAL(activeChanged(bool)), this, SLOT(saveFeedList() ) );
		//autoRefresh
		connect(feeds.at(index), SIGNAL(autoRefreshChanged(const QTime&)), this, SLOT(saveFeedList() ) );
		//ignoreTTL
		connect(feeds.at(index), SIGNAL(ignoreTTLChanged(bool)), this, SLOT(saveFeedList() ) );
	
	}
	
	void RssFeedManager::addNewAcceptFilter(RssFilter filter)
	{
		if (acceptFilters.isEmpty())
			deleteAcceptFilter->setEnabled(true);
		
		acceptFilters.append(new RssFilter(filter));
		
		int index = acceptFilters.count()-1;
		acceptFilterList->insertItem(acceptFilters.at(index)->title());
		acceptFilterList->setCurrentItem(index);
		
		connect(acceptFilters.at(index), SIGNAL(titleChanged(const QString&)), this, SLOT(updateAcceptFilterList()) );
		
		//connect all the fields to the save slot
		//title
		connect(acceptFilters.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(saveFilterList() ) );
		//active
		connect(acceptFilters.at(index), SIGNAL(activeChanged( bool )), this, SLOT(saveFilterList() ) );
		//regexps
		connect(acceptFilters.at(index), SIGNAL(regExpsChanged( const QStringList& )), this, SLOT(saveFilterList() ) );
		//series
		connect(acceptFilters.at(index), SIGNAL(seriesChanged( bool )), this, SLOT(saveFilterList() ) );
		//sansEpisode
		connect(acceptFilters.at(index), SIGNAL(sansEpisodeChanged( bool )), this, SLOT(saveFilterList() ) );
		//minSeason
		connect(acceptFilters.at(index), SIGNAL(minSeasonChanged (int )), this, SLOT(saveFilterList() ) );
		//minEpisode
		connect(acceptFilters.at(index), SIGNAL(minEpisodeChanged (int )), this, SLOT(saveFilterList() ) );
		//maxSeason
		connect(acceptFilters.at(index), SIGNAL(maxSeasonChanged (int )), this, SLOT(saveFilterList() ) );
		//maxEpiosde
		connect(acceptFilters.at(index), SIGNAL(maxEpisodeChanged (int )), this, SLOT(saveFilterList() ) );
		//matches
		connect(acceptFilters.at(index), SIGNAL(matchesChanged( const QValueList<FilterMatch>& )), this, SLOT(saveFilterList() ) );
		
		//connect the rescan signal to the rescan slot
		connect(acceptFilters.at(index), SIGNAL(rescanFilter()), this, SLOT(rescanFilter()) );
		
// 		//connect all except the matchesChanged to the rescanFilter slot
// 		//title
// 		connect(acceptFilters.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(rescanFilter() ) );
// 		//active
// 		connect(acceptFilters.at(index), SIGNAL(activeChanged( bool )), this, SLOT(rescanFilter() ) );
// 		//regexps
// 		connect(acceptFilters.at(index), SIGNAL(regExpsChanged( const QStringList& )), this, SLOT(rescanFilter() ) );
// 		//series
// 		connect(acceptFilters.at(index), SIGNAL(seriesChanged( bool )), this, SLOT(rescanFilter() ) );
// 		//sansEpisode
// 		connect(acceptFilters.at(index), SIGNAL(sansEpisodeChanged( bool )), this, SLOT(rescanFilter() ) );
// 		//minSeason
// 		connect(acceptFilters.at(index), SIGNAL(minSeasonChanged (int )), this, SLOT(rescanFilter() ) );
// 		//minEpisode
// 		connect(acceptFilters.at(index), SIGNAL(minEpisodeChanged (int )), this, SLOT(rescanFilter() ) );
// 		//maxSeason
// 		connect(acceptFilters.at(index), SIGNAL(maxSeasonChanged (int )), this, SLOT(rescanFilter() ) );
// 		//maxEpiosde
// 		connect(acceptFilters.at(index), SIGNAL(maxEpisodeChanged (int )), this, SLOT(rescanFilter() ) );

	}
	
	void RssFeedManager::addNewRejectFilter(RssFilter filter)
	{
		if (rejectFilters.isEmpty())
			deleteRejectFilter->setEnabled(true);
		
		rejectFilters.append(new RssFilter(filter));
		
		int index = rejectFilters.count()-1;
		rejectFilterList->insertItem(rejectFilters.at(index)->title());
		rejectFilterList->setCurrentItem(index);
		
		connect(rejectFilters.at(index), SIGNAL(titleChanged(const QString&)), this, SLOT(updateRejectFilterList()) );
		
		//connect all the fields to the save slot
		//title
		connect(rejectFilters.at(index), SIGNAL(titleChanged(const QString &)), this, SLOT(saveFilterList() ) );
		//active
		connect(rejectFilters.at(index), SIGNAL(activeChanged( bool )), this, SLOT(saveFilterList() ) );
		//regexps
		connect(rejectFilters.at(index), SIGNAL(regExpsChanged( const QStringList& )), this, SLOT(saveFilterList() ) );
		//series
		connect(rejectFilters.at(index), SIGNAL(seriesChanged( bool )), this, SLOT(saveFilterList() ) );
		//sansEpisode
		connect(rejectFilters.at(index), SIGNAL(sansEpisodeChanged( bool )), this, SLOT(saveFilterList() ) );
		//minSeason
		connect(rejectFilters.at(index), SIGNAL(minSeasonChanged (int )), this, SLOT(saveFilterList() ) );
		//minEpisode
		connect(rejectFilters.at(index), SIGNAL(minEpisodeChanged (int )), this, SLOT(saveFilterList() ) );
		//maxSeason
		connect(rejectFilters.at(index), SIGNAL(maxSeasonChanged (int )), this, SLOT(saveFilterList() ) );
		//maxEpiosde
		connect(rejectFilters.at(index), SIGNAL(maxEpisodeChanged (int )), this, SLOT(saveFilterList() ) );
		//matches
		connect(rejectFilters.at(index), SIGNAL(matchesChanged( const QValueList<FilterMatch>& )), this, SLOT(saveFilterList() ) );

	}
	
	void RssFeedManager::deleteSelectedFeed()
	{
		int currentItem = feedlist->currentItem();
		
		if (currentItem < 0)
			return;
			
		int newItem=currentItem-1;
		
		if (currentItem == -1 && feeds.count())
			newItem = 0;
		
		disconnectFeed(currentItem);
		currentFeed = -1;
		
		delete feeds.at(currentItem);
		feeds.remove(currentItem);
		feedlist->removeItem(currentItem);
		
		if (feeds.isEmpty())
			deleteFeed->setEnabled(false);
		
		if (newItem >= 0)
		{
			feedlist->setSelected(newItem, true);
		}
		
		saveFeedList();
	}
	
	void RssFeedManager::deleteSelectedAcceptFilter()
	{
		int currentItem = acceptFilterList->currentItem();
		
		if (currentItem < 0)
			return;
		
		int newItem=currentItem-1;
		
		if (currentItem == -1 && acceptFilters.count())
			newItem = 0;
		
		disconnectFilter(currentItem, true);
		currentAcceptFilter = -1;
		
		delete acceptFilters.at(currentItem);
		acceptFilters.remove(currentItem);
		acceptFilterList->removeItem(currentItem);
		
		if (acceptFilters.isEmpty())
			deleteAcceptFilter->setEnabled(false);
		
		if (newItem >= 0)
		{
			acceptFilterList->setSelected(newItem, true);
		}
		
		saveFilterList();
	}
	
	void RssFeedManager::deleteSelectedRejectFilter()
	{
		int currentItem = rejectFilterList->currentItem();
		
		if (currentItem < 0)
			return;
		
		int newItem=currentItem-1;
		
		if (currentItem == -1 && rejectFilters.count())
			newItem = 0;
		
		disconnectFilter(currentItem, false);
		currentRejectFilter = -1;
		
		delete rejectFilters.at(currentItem);
		rejectFilters.remove(currentItem);
		rejectFilterList->removeItem(currentItem);
		
		if (rejectFilters.isEmpty())
			deleteRejectFilter->setEnabled(false);
		
		if (newItem >= 0)
		{
			rejectFilterList->setSelected(newItem, true);
		}
		
		saveFilterList();
	}
	
	void RssFeedManager::updateRegExps()
	{
		if (currentRejectFilter < 0)
		{
			//accept filter is active
			acceptFilters.at(currentAcceptFilter)->setRegExps(filterRegExps->items());
		}
		else
		{
			//reject filter is active
			rejectFilters.at(currentRejectFilter)->setRegExps(filterRegExps->items());
		}
	}
	
	void RssFeedManager::updateFeedList(int item)
	{
		int cursorPos = feedTitle->cursorPosition();
		if (item < 0)
		{
			//let's check which one sent the signal - if we can't figure it all then update them all
			int pos = feeds.find((RssFeed *)sender());
			
			if (pos < 0)
			{
				for (int i=0; i<feedlist->count(); i++)
				{
					feedlist->changeItem(feeds.at(i)->title(), i);
				}
			}
			else
			{
				//just change the feed sending the signal
				feedlist->changeItem(feeds.at(pos)->title(), pos);
				if (feedlist->isSelected(pos))
				{
					feedTitle->setFocus();
				}
			}
		}
		else
		{
			//just update item
			feedlist->changeItem(feeds.at(item)->title(), item);
		}
		feedTitle->setCursorPosition(cursorPos);
	}
	
	void RssFeedManager::updateAcceptFilterList(int item)
	{
		int cursorPos = filterTitle->cursorPosition();
		if (item < 0)
		{
			//let's check which one sent the signal - if we can't figure it all then update them all
			int pos = acceptFilters.find((RssFilter *)sender());
			
			if (pos < 0)
			{
				for (int i=0; i<feedlist->count(); i++)
				{
					acceptFilterList->changeItem(acceptFilters.at(i)->title(), i);
				}
			}
			else
			{
				//just change the feed sending the signal
				acceptFilterList->changeItem(acceptFilters.at(pos)->title(), pos);
				if (acceptFilterList->isSelected(pos))
				{
					filterTitle->setFocus();
				}
			}
		}
		else
		{
			//just update item
			acceptFilterList->changeItem(acceptFilters.at(item)->title(), item);
		}
		filterTitle->setCursorPosition(cursorPos);
	}
	
	void RssFeedManager::updateRejectFilterList(int item)
	{
		int cursorPos = filterTitle->cursorPosition();
		if (item < 0)
		{
			//let's check which one sent the signal - if we can't figure it all then update them all
			int pos = rejectFilters.find((RssFilter *)sender());
			
			if (pos < 0)
			{
				for (int i=0; i<feedlist->count(); i++)
				{
					rejectFilterList->changeItem(rejectFilters.at(i)->title(), i);
				}
			}
			else
			{
				//just change the feed sending the signal
				rejectFilterList->changeItem(rejectFilters.at(pos)->title(), pos);
				if (rejectFilterList->isSelected(pos))
				{
					filterTitle->setFocus();
				}
			}
		}
		else
		{
			//just update item
			rejectFilterList->changeItem(rejectFilters.at(item)->title(), item);
		}
		filterTitle->setCursorPosition(cursorPos);
	}
	
	void RssFeedManager::updateArticles(const RssArticle::List& articles)
	{
		feedArticles->setNumRows(articles.count());
		for (int i=0; i<articles.count(); i++)
			{
			QString info;
			if (articles[i].downloaded()==1)
				{
				info = ": Manually downloaded";
				}
			else if (articles[i].downloaded()==3)
				{
				info = ": Automatically downloaded";
				}
			feedArticles->setText(i, 0, articles[i].title() + info);
			feedArticles->setText(i, 1, articles[i].description());
			feedArticles->setText(i, 2, articles[i].link().prettyURL());
			}
	}
	
	void RssFeedManager::updateMatches(const QValueList<FilterMatch>& matches)
	{
		filterMatches->setNumRows(matches.count());
		for (int i=0; i<matches.count(); i++)
			{
			filterMatches->setText(i, 0, QString::number(matches[i].season()));
			filterMatches->setText(i, 1, QString::number(matches[i].episode()));
			filterMatches->setText(i, 2, matches[i].time());
			filterMatches->setText(i, 3, matches[i].link());
			}
		
		changedMatchSelection();
	}
	
	void RssFeedManager::changedArticleSelection()
	{
		bool downloadEnabled = false;
		for (int i=0; i<feedArticles->numSelections(); i++)
		{
			if (feedArticles->selection(i).numRows())
			{
				downloadEnabled = true;
				break;
			}
		}
		downloadArticle->setEnabled(downloadEnabled);
	}
	
	void RssFeedManager::changedMatchSelection()
	{
		bool downloadEnabled = false;
		for (int i=0; i<filterMatches->numSelections(); i++)
		{
			if (filterMatches->selection(i).numRows())
			{
				downloadEnabled = true;
				break;
			}
		}
		downloadFilterMatch->setEnabled(downloadEnabled);
		deleteFilterMatch->setEnabled(downloadEnabled);
	}
	
	void RssFeedManager::downloadSelectedArticles()
	{
		for (int i=0; i<feedArticles->numSelections(); i++)
		{
			int endRow = feedArticles->selection(i).topRow() + feedArticles->selection(i).numRows(); 
			RssLinkDownloader * curDownload;
			for (int j=feedArticles->selection(i).topRow(); j<endRow; j++)
			{
				curDownload = new RssLinkDownloader(m_core, feedArticles->text(j, 2));
				for (int i=0; i<feeds.count(); i++)
					{
					connect(curDownload, SIGNAL(linkDownloaded( QString, int )), feeds.at(i), SLOT(setDownloaded(QString, int)) );
					}
			}
		}
	}
	
	void RssFeedManager::downloadSelectedMatches()
	{
		for (int i=0; i<filterMatches->numSelections(); i++)
		{
			int endRow = filterMatches->selection(i).topRow() + filterMatches->selection(i).numRows(); 
			for (int j=filterMatches->selection(i).topRow(); j<endRow; j++)
			{
				new RssLinkDownloader(m_core, filterMatches->text(j, 3));
			}
		}
	}
	
	void RssFeedManager::deleteSelectedMatches()
	{
		QStringList selectedLinks;
		for (int i=0; i<filterMatches->numSelections(); i++)
		{
			int endRow = filterMatches->selection(i).topRow() + filterMatches->selection(i).numRows(); 
			for (int j=filterMatches->selection(i).topRow(); j<endRow; j++)
			{
				//add a match to the list of selected matches
				selectedLinks.append(filterMatches->text(j, 3));
			}
		}
		
		RssFilter * curFilter;
		if (currentRejectFilter<0)
		{
			//we're currently testing an acceptFilter
			curFilter = acceptFilters.at(currentAcceptFilter);
		}
		else
		{
			//it's a reject filter
			curFilter = rejectFilters.at(currentRejectFilter);
		}
		
		for (int i=0; i<selectedLinks.count(); i++)
		{
			curFilter->deleteMatch( selectedLinks[i] );
		}
		
		updateMatches(curFilter->matches());
	}
	
	void RssFeedManager::changedActiveFeed()
	{
		if (currentFeed != feedlist->currentItem() || currentFeed < 0)
		{
			//the selection has indeed changed
			if (currentFeed >= 0)
			{
				//disconnect the gui signals from the old feed
				disconnectFeed(currentFeed);
			}
			
			//update the currentFeed
			currentFeed = feedlist->currentItem();
			if (currentFeed >= 0)
			{
				//set the values
				//title
				feedTitle->setText(feeds.at(currentFeed)->title());
				//url
				feedUrl->setKURL(feeds.at(currentFeed)->feedUrl());
				refreshFeed->setEnabled(!feeds.at(currentFeed)->feedUrl().url().isEmpty());
				//articleAge
				feedArticleAge->setValue(feeds.at(currentFeed)->articleAge());
				//active
				feedActive->setChecked(feeds.at(currentFeed)->active());
				//autoRefresh
				feedAutoRefresh->setTime(feeds.at(currentFeed)->autoRefresh());
				//ignoreTTL
				feedIgnoreTTL->setChecked(feeds.at(currentFeed)->ignoreTTL());
				feedAutoRefresh->setEnabled(feeds.at(currentFeed)->ignoreTTL());
				//articles
				updateArticles(feeds.at(currentFeed)->articles());
				
				//title
				feedTitle->setEnabled(true);
				//url
				feedUrl->setEnabled(true);
				//articleAge
				feedArticleAge->setEnabled(true);
				//active
				feedActive->setEnabled(true);
				//ignoreTTL
				feedIgnoreTTL->setEnabled(true);
				
				//connect all the signals
				connectFeed(currentFeed);
			}
			else
			{
				//clear the items
				//title
				feedTitle->clear();
				//url
				feedUrl->clear();
				//articleAge
				feedArticleAge->setValue(0);
				//active
				feedActive->setChecked(false);
				//autoRefresh
				feedAutoRefresh->setTime(QTime());
				//ignoreTTL
				feedIgnoreTTL->setChecked(false);
				//articles
				feedArticles->setNumRows(0);
				
				//title
				feedTitle->setEnabled(false);
				//url
				feedUrl->setEnabled(false);
				//articleAge
				feedArticleAge->setEnabled(false);
				//active
				feedActive->setEnabled(false);
				//autoRefresh
				feedAutoRefresh->setEnabled(false);
				//ignoreTTL
				feedIgnoreTTL->setEnabled(false);
			}
		}
	}
	
	void RssFeedManager::changedActiveAcceptFilter()
	{
		if (currentRejectFilter >= 0)
			{
			rejectFilterList->setSelected(currentRejectFilter, false);
			disconnectFilter(currentRejectFilter, false);
			currentRejectFilter = -1;
			}
		
		if (currentAcceptFilter != acceptFilterList->currentItem() || currentAcceptFilter < 0)
		{
			//the selection has indeed changed
			
			if (currentAcceptFilter >= 0)
			{
				//disconnect the gui signals from the old feed
				disconnectFilter(currentAcceptFilter, true);
				
			}
			
			//update the currentFeed
			currentAcceptFilter = acceptFilterList->currentItem();
			
			if (currentAcceptFilter >= 0)
			{
				//set the values
				filterTitle->setText(acceptFilters.at(currentAcceptFilter)->title());
				filterActive->setChecked(acceptFilters.at(currentAcceptFilter)->active());
				filterRegExps->setItems(acceptFilters.at(currentAcceptFilter)->regExps());
				filterSeries->setChecked(acceptFilters.at(currentAcceptFilter)->series());
				filterSansEpisode->setChecked(acceptFilters.at(currentAcceptFilter)->sansEpisode());
				filterMinSeason->setValue(acceptFilters.at(currentAcceptFilter)->minSeason());
				filterMinEpisode->setValue(acceptFilters.at(currentAcceptFilter)->minEpisode());
				filterMaxSeason->setValue(acceptFilters.at(currentAcceptFilter)->maxSeason());
				filterMaxEpisode->setValue(acceptFilters.at(currentAcceptFilter)->maxEpisode());
				
				updateMatches(acceptFilters.at(currentAcceptFilter)->matches());
				
				filterTitle->setEnabled(true);
				filterActive->setEnabled(true);
				filterRegExps->setEnabled(true);
				filterSeries->setEnabled(true);
				filterSansEpisode->setEnabled(true);
				filterMinSeason->setEnabled(true);
				filterMinEpisode->setEnabled(true);
				filterMaxSeason->setEnabled(true);
				filterMaxEpisode->setEnabled(true);
				
				processFilter->setEnabled(true);
				testText->setEnabled(true);
				
				//connect all the signals
				connectFilter(currentAcceptFilter, true);
			}
			else
			{
				if (currentRejectFilter < 0)
				{
					//clear the items
					filterTitle->clear();
					filterActive->setChecked(false);
					filterRegExps->clear();
					filterSeries->setChecked(false);
					filterSansEpisode->setChecked(false);
					filterMinSeason->setValue(0);
					filterMinEpisode->setValue(0);
					filterMaxSeason->setValue(0);
					filterMaxEpisode->setValue(0);
					filterMatches->setNumRows(0);
					
					filterTitle->setEnabled(false);
					filterActive->setEnabled(false);
					filterRegExps->setEnabled(false);
					filterSeries->setEnabled(false);
					filterSansEpisode->setEnabled(false);
					filterMinSeason->setEnabled(false);
					filterMinEpisode->setEnabled(false);
					filterMaxSeason->setEnabled(false);
					filterMaxEpisode->setEnabled(false);
					
					processFilter->setEnabled(false);
					testText->setEnabled(false);
					
				}
			}
		}
	}
	
	void RssFeedManager::changedActiveRejectFilter()
	{
		if (currentAcceptFilter >= 0)
		{
			acceptFilterList->setSelected(currentAcceptFilter, false);
			disconnectFilter(currentAcceptFilter, true);
			currentAcceptFilter = -1;
		}
		
		if (currentRejectFilter != rejectFilterList->currentItem() || currentRejectFilter < 0)
		{
			//the selection has indeed changed
			
			if (currentRejectFilter >= 0)
			{
				//disconnect the gui signals from the old feed
				disconnectFilter(currentRejectFilter, false);
			}
			
			//update the currentFeed
			currentRejectFilter = rejectFilterList->currentItem();
			
			if (currentRejectFilter >= 0)
			{
				//set the values
				//title
				filterTitle->setText(rejectFilters.at(currentRejectFilter)->title());
				filterActive->setChecked(rejectFilters.at(currentRejectFilter)->active());
				filterRegExps->setItems(rejectFilters.at(currentRejectFilter)->regExps());
				filterSeries->setChecked(rejectFilters.at(currentRejectFilter)->series());
				filterSansEpisode->setChecked(rejectFilters.at(currentRejectFilter)->sansEpisode());
				filterMinSeason->setValue(rejectFilters.at(currentRejectFilter)->minSeason());
				filterMinEpisode->setValue(rejectFilters.at(currentRejectFilter)->minEpisode());
				filterMaxSeason->setValue(rejectFilters.at(currentRejectFilter)->maxSeason());
				filterMaxEpisode->setValue(rejectFilters.at(currentRejectFilter)->maxEpisode());
				
				updateMatches(rejectFilters.at(currentRejectFilter)->matches());
				
				filterTitle->setEnabled(true);
				filterActive->setEnabled(true);
				filterRegExps->setEnabled(true);
				filterSeries->setEnabled(true);
				filterSansEpisode->setEnabled(true);
				filterMinSeason->setEnabled(true);
				filterMinEpisode->setEnabled(true);
				filterMaxSeason->setEnabled(true);
				filterMaxEpisode->setEnabled(true);
				
				processFilter->setEnabled(true);
				testText->setEnabled(true);
				
				//connect all the signals
				connectFilter(currentRejectFilter, false);
			}
			else
			{
				if (currentRejectFilter < 0)
				{
					//clear the items
					filterTitle->clear();
					filterActive->setChecked(false);
					filterRegExps->clear();
					filterSeries->setChecked(false);
					filterSansEpisode->setChecked(false);
					filterMinSeason->setValue(0);
					filterMinEpisode->setValue(0);
					filterMaxSeason->setValue(0);
					filterMaxEpisode->setValue(0);
					filterMatches->setNumRows(0);
					
					filterTitle->setEnabled(false);
					filterActive->setEnabled(false);
					filterRegExps->setEnabled(false);
					filterSeries->setEnabled(false);
					filterSansEpisode->setEnabled(false);
					filterMinSeason->setEnabled(false);
					filterMinEpisode->setEnabled(false);
					filterMaxSeason->setEnabled(false);
					filterMaxEpisode->setEnabled(false);
					
					processFilter->setEnabled(false);
					testText->setEnabled(false);
				}
			}
		}
	}
	
	QString RssFeedManager::getFeedListFilename()
	{
		return KGlobal::dirs()->saveLocation("data","ktorrent") + "rssfeeds.ktr";
	}
	
	QString RssFeedManager::getFilterListFilename()
	{
		return KGlobal::dirs()->saveLocation("data","ktorrent") + "rssfilters.ktr";
	}
	
	void RssFeedManager::saveFeedList()
	{
		if (feedListSaving)
			return;
			
		feedListSaving = true;
		
		QString filename = getFeedListFilename();
		
		//save feeds to disk
		QFile file(filename);
		
		file.open( IO_WriteOnly );
		QDataStream out(&file);
		
		out << feeds.count();
		
		for (int i=0; i<feeds.count(); i++)
		{
			out << *(feeds.at(i));
		}
		
		feedListSaving = false;
	}

	void RssFeedManager::loadFeedList()
	{
		QString filename = getFeedListFilename();
		
		//load feeds from disk
		QFile file(filename);
		
		if (file.exists())
			{
				file.open( IO_ReadOnly );
				QDataStream in(&file);
				
				int numFeeds;
				
				in >> numFeeds;
				
				RssFeed curFeed;
				
				for (int i=0; i<numFeeds; i++)
				{
					in >> curFeed;
					addNewFeed(curFeed);
				}
				
				changedActiveFeed();
			}	
	}

	void RssFeedManager::saveFilterList()
	{
		if (filterListSaving)
			return;
		
		filterListSaving = true;
		
		QString filename = getFilterListFilename();
		
		//save feeds to disk
		QFile file(filename);
		
		file.open( IO_WriteOnly );
		QDataStream out(&file);
		
		out << acceptFilters.count();
		
		for (int i=0; i<acceptFilters.count(); i++)
		{
			out << *(acceptFilters.at(i));
		}
	
		out << rejectFilters.count();
		
		for (int i=0; i<rejectFilters.count(); i++)
		{
			out << *(rejectFilters.at(i));
		}
		
		filterListSaving = false;
	}

	void RssFeedManager::loadFilterList()
	{
		QString filename = getFilterListFilename();
		
		//load feeds from disk
		QFile file(filename);
		
		if (file.exists())
			{
				file.open( IO_ReadOnly );
				QDataStream in(&file);
				
				int numFilters;
				
				in >> numFilters;
				
				RssFilter curFilter;
				
				for (int i=0; i<numFilters; i++)
				{
					in >> curFilter;
					addNewAcceptFilter(curFilter);
				}
				
				in >> numFilters;
				
				for (int i=0; i<numFilters; i++)
				{
					in >> curFilter;
					addNewRejectFilter(curFilter);
				}
								
				//go through and grab the reject filters
				changedActiveRejectFilter();
				changedActiveAcceptFilter();
			}	
	}
	
	void RssFeedManager::scanArticle(RssArticle article, RssFilter * filter)
	{
		//first run it through the reject filters - if any match go no further
		for (int i=0; i<rejectFilters.count(); i++)
		{
			if (rejectFilters.at(i)->scanArticle(article, false))
			{
				return;
			}
		}
		
		if (filter)
		{
			//we were passed a filter - so just scan it with that one
			if (filter->scanArticle(article))
			{
				RssLinkDownloader * curDownload = new RssLinkDownloader(m_core, article.link().prettyURL(), filter);
				for (int i=0; i<feeds.count(); i++)
					{
					connect(curDownload, SIGNAL(linkDownloaded( QString, int )), feeds.at(i), SLOT(setDownloaded(QString, int)) );
					}
			}
		}
		else
		{
			for (int i=0; i<acceptFilters.count(); i++)
			{
				if (acceptFilters.at(i)->scanArticle(article))
				{
				RssLinkDownloader * curDownload = new RssLinkDownloader(m_core, article.link().prettyURL(), acceptFilters.at(i));
				for (int i=0; i<feeds.count(); i++)
					{
					connect(curDownload, SIGNAL(linkDownloaded( QString, int )), feeds.at(i), SLOT(setDownloaded(QString, int)) );
					}
				}
			}
		}
	}
	
	void RssFeedManager::rescanFilter()
	{
		int pos = acceptFilters.find((RssFilter *)sender());
		
		if (pos >= 0)
		{
			for (int i=0; i<feeds.count(); i++)
			{
				for (int j=0; j<feeds.at(i)->articles().count(); j++)
				{
					scanArticle(feeds.at(i)->articles()[j], (RssFilter *)sender());
				}
			}
		}
	}
	
	void RssFeedManager::testTextChanged()
	{
		testText->setPaletteBackgroundColor(QColor(255, 255, 255));
		testTestText->setEnabled(!testText->text().isEmpty());
	}
	
	void RssFeedManager::testFilter()
	{
		RssFilter * curFilter;
		if (currentRejectFilter<0)
		{
			//we're currently testing an acceptFilter
			curFilter = acceptFilters.at(currentAcceptFilter);
		}
		else
		{
			//it's a reject filter
			curFilter = rejectFilters.at(currentRejectFilter);
		}
		
		RssArticle testArticle;
		testArticle.setTitle(testText->text());
		
		if (curFilter->scanArticle(testArticle, false, false))
		{
		testText->setPaletteBackgroundColor(QColor(0, 255, 0));
		}
		else
		{
		testText->setPaletteBackgroundColor(QColor(255, 0, 0));
		}
	}
	
	void RssFeedManager::setFilterTitle(const QString& title)
	{
		int cursorPos = filterTitle->cursorPosition();
		filterTitle->setText(title);
		filterTitle->setCursorPosition(cursorPos);
	}
	
	void RssFeedManager::setFeedTitle(const QString& title)
	{
		int cursorPos = feedTitle->cursorPosition();
		feedTitle->setText(title);
		feedTitle->setCursorPosition(cursorPos);
	}

}
