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
#include <QInputDialog>
#include <QToolButton>

#include <KMainWindow>
#include <KMessageBox>

#include <magnet/magnetlink.h>
#include <interfaces/functions.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <util/error.h>
#include <util/fileops.h>
#include <syndication/loader.h>
#include "syndicationactivity.h"
#include "feedwidget.h"
#include "ktfeed.h"
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
#include "feedretriever.h"


namespace kt
{
    SyndicationActivity::SyndicationActivity(SyndicationPlugin* sp, QWidget* parent)
        : Activity(i18n("Syndication"), QStringLiteral("application-rss+xml"), 30, parent), sp(sp)
    {
        QString ddir = kt::DataDir() + QStringLiteral("syndication/");
        if (!bt::Exists(ddir))
            bt::MakeDir(ddir, true);

        setToolTip(i18n("Manages RSS and Atom feeds"));
        QHBoxLayout* layout = new QHBoxLayout(this);
        splitter = new QSplitter(Qt::Horizontal, this);
        layout->addWidget(splitter);

        feed_list = new FeedList(ddir, this);
        filter_list = new FilterList(this);
        tab = new SyndicationTab(sp->actionCollection(), feed_list, filter_list, splitter);
        splitter->addWidget(tab);

        feed_widget = new FeedWidget(filter_list, this, splitter);
        splitter->addWidget(feed_widget);
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 3);

        connect(tab->feedView(), &kt::FeedListView::feedActivated, this, &SyndicationActivity::showFeed);
        connect(tab->feedView(), &kt::FeedListView::enableRemove, sp->remove_feed, &QAction::setEnabled);
        connect(tab->feedView(), &kt::FeedListView::enableRemove, sp->manage_filters, &QAction::setEnabled);
        connect(tab->filterView(), &kt::FilterListView::filterActivated, this, static_cast<void(SyndicationActivity::*)(Filter*)>(&SyndicationActivity::editFilter));
        connect(tab->filterView(), &kt::FilterListView::enableRemove, sp->remove_filter, &QAction::setEnabled);
        connect(tab->filterView(), &kt::FilterListView::enableEdit, sp->edit_filter, &QAction::setEnabled);

        filter_list->loadFilters(kt::DataDir() + QStringLiteral("syndication/filters"));
        feed_list->loadFeeds(filter_list, this);
        feed_list->importOldFeeds();
    }

    SyndicationActivity::~SyndicationActivity()
    {
    }

    void SyndicationActivity::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("SyndicationActivity");
        QString current = g.readEntry("current_feed", QString());

        Feed* f = feed_list->feedForDirectory(current);
        if (f)
            feed_widget->setFeed(f);

        QByteArray state = g.readEntry("splitter", QByteArray());
        splitter->restoreState(state);
        tab->loadState(g);
        feed_widget->loadState(g);
    }

    void SyndicationActivity::saveState(KSharedConfigPtr cfg)
    {
        Feed* feed = feed_widget->getFeed();

        KConfigGroup g = cfg->group("SyndicationActivity");
        g.writeEntry("current_feed", feed ? feed->directory() : QString());
        g.writeEntry("splitter", splitter->saveState());
        tab->saveState(g);
        feed_widget->saveState(g);
        g.sync();
    }

    void SyndicationActivity::addFeed()
    {
        bool ok = false;
        QString url = QInputDialog::getText(sp->getGUI()->getMainWindow(), i18n("Enter the URL"),
                                            i18n("Please enter the URL of the RSS or Atom feed."),
                                            QLineEdit::Normal, QString(), &ok);
        if (!ok || url.isEmpty())
            return;

        Syndication::Loader* loader = Syndication::Loader::create(this, SLOT(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
        QStringList sl = url.split(QStringLiteral(":COOKIE:"));
        if (sl.size() == 2)
        {
            FeedRetriever* retr = new FeedRetriever();
            retr->setAuthenticationCookie(sl.last());
            loader->loadFrom(QUrl(sl.first()), retr);
            downloads.insert(loader, url);
        }
        else
        {
            loader->loadFrom(QUrl(url));
            downloads.insert(loader, url);
        }
    }

    void SyndicationActivity::loadingComplete(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode status)
    {
        if (status != Syndication::Success)
        {
            QString error = SyndicationErrorString(status);
            KMessageBox::error(tab, i18n("Failed to load feed %1: %2", downloads[loader], error));
            downloads.remove(loader);
            return;
        }

        try
        {
            QString ddir = kt::DataDir() + QStringLiteral("syndication/");
            Feed* f = new Feed(downloads[loader], feed, Feed::newFeedDir(ddir));
            connect(f, &Feed::downloadLink, this, &SyndicationActivity::downloadLink);
            f->save();
            feed_list->addFeed(f);
            feed_widget->setFeed(f);
        }
        catch (bt::Error& err)
        {
            KMessageBox::error(tab, i18n("Failed to create directory for feed %1: %2", downloads[loader], err.toString()));
        }
        downloads.remove(loader);
    }

    void SyndicationActivity::removeFeed()
    {
        QModelIndexList idx = tab->feedView()->selectedFeeds();
        foreach (const QModelIndex& i, idx)
        {
            Feed* f = feed_list->feedForIndex(i);
            if (f && feed_widget->getFeed() == f)
            {
                feed_widget->setFeed(0);
            }
        }
        feed_list->removeFeeds(idx);
    }

    void SyndicationActivity::showFeed(Feed* f)
    {
        if (!f)
            return;

        feed_widget->setFeed(f);
    }


    void SyndicationActivity::downloadLink(const QUrl& url,
                                           const QString& group,
                                           const QString& location,
                                           const QString& move_on_completion,
                                           bool silently)
    {
        if (url.scheme() == QStringLiteral("magnet"))
        {
            MagnetLinkLoadOptions options;
            options.silently = silently;
            options.group = group;
            options.location = location;
            options.move_on_completion = move_on_completion;
            sp->getCore()->load(bt::MagnetLink(url), options);
        }
        else
        {
            LinkDownloader* dlr = new LinkDownloader(url, sp->getCore(), !silently, group, location, move_on_completion);
            dlr->start();
        }
    }

    Filter* SyndicationActivity::addNewFilter()
    {
        Filter* filter = new Filter(i18n("New Filter"));
        FilterEditor dlg(filter, filter_list, feed_list, sp->getCore(), sp->getGUI()->getMainWindow());
        dlg.setWindowTitle(i18n("Add New Filter"));
        if (dlg.exec() == QDialog::Accepted)
        {
            filter_list->addFilter(filter);
            filter_list->saveFilters(kt::DataDir() + QStringLiteral("syndication/filters"));
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
        const QModelIndexList indexes = tab->filterView()->selectedFilters();
        QList<Filter*> to_remove;
        for (const QModelIndex& idx : indexes)
        {
            Filter* f = filter_list->filterForIndex(idx);
            if (f)
                to_remove.append(f);
        }

        foreach (Filter* f, to_remove)
        {
            feed_list->filterRemoved(f);
            filter_list->removeFilter(f);
            delete f;
        }

        filter_list->saveFilters(kt::DataDir() + QStringLiteral("syndication/filters"));
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
        FilterEditor dlg(f, filter_list, feed_list, sp->getCore(), sp->getGUI()->getMainWindow());
        if (dlg.exec() == QDialog::Accepted)
        {
            filter_list->filterEdited(f);
            filter_list->saveFilters(kt::DataDir() + QStringLiteral("syndication/filters"));
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

        ManageFiltersDlg dlg(f, filter_list, this, tab);
        if (dlg.exec() == QDialog::Accepted)
        {
            f->save();
            f->runFilters();
        }
    }

    void SyndicationActivity::editFeedName()
    {
        QModelIndexList idx = tab->feedView()->selectedFeeds();
        if (idx.count())
            tab->feedView()->edit(idx.front());
    }
}
