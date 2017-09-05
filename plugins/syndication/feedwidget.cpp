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

#include <QApplication>
#include <QColor>
#include <QHeaderView>
#include <QInputDialog>
#include <QLocale>
#include <QPalette>

#include <KRun>

#include <util/log.h>
#include "ktfeed.h"
#include "feedwidget.h"
#include "feedwidgetmodel.h"
#include "managefiltersdlg.h"
#include "filterlist.h"
#include "syndicationplugin.h"

using namespace bt;

namespace kt
{
    QString FeedWidget::item_template = i18n("\
    <html>\
    <body style=\"color:%4\">\
    <div style=\"border-style:solid; border-width:1px; border-color:%4; margin:5px; padding:5px\">\
    <b>Title:</b> %1<br/>\
    <b>Date:</b> %2<br/>\
    </div>\
    <p>%3</p>\
    </body>\
    </html>\
    ");

    FeedWidget::FeedWidget(FilterList* filters, SyndicationActivity* act, QWidget* parent)
        : QWidget(parent),
          feed(nullptr),
          filters(filters),
          act(act)
    {
        setupUi(this);
        m_splitter->setStretchFactor(0, 3);
        m_splitter->setStretchFactor(1, 1);

        connect(m_download, &QPushButton::clicked, this, &FeedWidget::downloadClicked);
        connect(m_refresh, &QPushButton::clicked, this, &FeedWidget::refreshClicked);
        connect(m_filters, &QPushButton::clicked, this, &FeedWidget::filtersClicked);
        connect(m_refresh_rate, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &FeedWidget::refreshRateChanged);
        connect(m_cookies, &QPushButton::clicked, this, &FeedWidget::cookiesClicked);

        m_refresh->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
        m_filters->setIcon(QIcon::fromTheme(QStringLiteral("view-filter")));
        m_cookies->setIcon(QIcon::fromTheme(QStringLiteral("preferences-web-browser-cookies")));
        m_download->setIcon(QIcon::fromTheme(QStringLiteral("ktorrent")));

        model = new FeedWidgetModel(this);
        m_item_list->setModel(model);
        m_item_list->setAlternatingRowColors(true);
        m_item_list->setSelectionMode(QAbstractItemView::ExtendedSelection);


        QHeaderView* hv = m_item_list->header();
        hv->setSectionResizeMode(QHeaderView::Interactive);
        connect(m_item_list->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FeedWidget::selectionChanged);

        m_download->setEnabled(false);
        m_url->clear();
        m_refresh_rate->clear();
        m_active_filters->clear();

        m_item_view->setEnabled(false);
        m_item_view->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
        connect(m_item_view, &KWebView::linkClicked, this, &FeedWidget::linkClicked);

        setEnabled(false);
    }

    FeedWidget::~FeedWidget()
    {
    }

    void FeedWidget::loadState(KConfigGroup& g)
    {
        m_splitter->restoreState(g.readEntry("feed_widget_splitter", QByteArray()));

        QHeaderView* hv = m_item_list->header();
        QByteArray state = g.readEntry("feed_widget_list_header", QByteArray());
        if (!state.isEmpty())
            hv->restoreState(state);
        else
            QTimer::singleShot(3000, this, &FeedWidget::resizeColumns);
    }

    void FeedWidget::saveState(KConfigGroup& g)
    {
        g.writeEntry("feed_widget_splitter", m_splitter->saveState());
        QHeaderView* hv = m_item_list->header();
        g.writeEntry("feed_widget_list_header", hv->saveState());
    }

    void FeedWidget::resizeColumns()
    {
        m_item_list->header()->resizeSections(QHeaderView::ResizeToContents);
    }


    void FeedWidget::setFeed(Feed* f)
    {
        if (feed)
        {
            disconnect(feed, &Feed::updated, this, &FeedWidget::updated);
            disconnect(feed, &Feed::feedRenamed, this, &FeedWidget::onFeedRenamed);
            feed = 0;
        }

        feed = f;
        setEnabled(feed != 0);
        model->setCurrentFeed(f);
        if (feed)
        {
            connect(feed, &Feed::updated, this, &FeedWidget::updated);
            connect(feed, &Feed::feedRenamed, this, &FeedWidget::onFeedRenamed);

            m_url->setText(QStringLiteral("<b>%1</b>").arg(feed->feedUrl().toDisplayString()));
            m_refresh_rate->setValue(feed->refreshRate());
            updated();
            selectionChanged(m_item_list->selectionModel()->selection(), QItemSelection());
        }
    }


    void FeedWidget::downloadClicked()
    {
        if (!feed)
            return;

        QModelIndexList sel = m_item_list->selectionModel()->selectedRows();
        foreach (const QModelIndex& idx, sel)
        {
            Syndication::ItemPtr ptr = model->itemForIndex(idx);
            if (ptr)
                feed->downloadItem(ptr, QString(), QString(), QString(), false);
        }
    }

    void FeedWidget::refreshClicked()
    {
        if (feed)
            feed->refresh();
    }

    void FeedWidget::refreshRateChanged(int v)
    {
        if (v > 0 && feed)
            feed->setRefreshRate(v);
    }

    void FeedWidget::filtersClicked()
    {
        if (!feed)
            return;

        ManageFiltersDlg dlg(feed, filters, act, this);
        if (dlg.exec() == QDialog::Accepted)
        {
            feed->save();
            feed->runFilters();
        }
    }

    void FeedWidget::cookiesClicked()
    {
        if (!feed)
            return;

        bool ok = false;
        QString cookie = feed->authenticationCookie();
        QString nc = QInputDialog::getText(0, i18n("Authentication Cookie"), i18n("Enter the new authentication cookie"), QLineEdit::Normal, cookie, &ok);
        if (ok)
        {
            feed->setAuthenticationCookie(nc);
            feed->save();
        }
    }

    void FeedWidget::selectionChanged(const QItemSelection& sel, const QItemSelection& prev)
    {
        Q_UNUSED(prev);
        m_download->setEnabled(sel.count() > 0);
        m_item_view->setEnabled(sel.count() > 0);
        if (sel.count() > 0 && feed)
        {
            Syndication::ItemPtr item = model->itemForIndex(m_item_list->selectionModel()->selectedRows().front());
            if (item)
            {
                m_item_view->setHtml(item_template
                                     .arg(item->title())
                                     .arg(QLocale().toString(QDateTime::fromTime_t(item->datePublished()), QLocale::ShortFormat))
                                     .arg(item->description())
                                     .arg(QApplication::palette().text().color().name(QColor::NameFormat::HexRgb))
                                    , QUrl(feed->feedData()->link()));
            }
        }
    }

    void FeedWidget::updated()
    {
        if (!feed)
            return;

        switch (feed->feedStatus())
        {
        case Feed::OK:
            m_status->setText(i18n("<b>OK</b>"));
            break;
        case Feed::UNLOADED:
            m_status->setText(i18n("<b>Not Loaded</b>"));
            break;
        case Feed::FAILED_TO_DOWNLOAD:
            m_status->setText(i18n("<b>Download Failed: %1</b>", feed->errorString()));
            break;
        case Feed::DOWNLOADING:
            m_status->setText(i18n("<b>Downloading</b>"));
            break;
        }
        updateCaption(this, feed->title());
        m_active_filters->setText(QStringLiteral("<b>") + feed->filterNamesString() + QStringLiteral("</b>"));
    }


    void FeedWidget::onFeedRenamed(kt::Feed* f)
    {
        updateCaption(this, f->displayName());
    }

    void FeedWidget::linkClicked(const QUrl& url)
    {
        Out(SYS_SYN | LOG_DEBUG) << "linkClicked " << url.toString() << endl;
        new KRun(url, QApplication::activeWindow());
    }


}
