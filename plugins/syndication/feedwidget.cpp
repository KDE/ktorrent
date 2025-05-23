/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QColor>
#include <QHeaderView>
#include <QInputDialog>
#include <QLocale>
#include <QPalette>
#include <QStyleHints>
#include <QWebEngineSettings>

#include "feedwidget.h"
#include "feedwidgetmodel.h"
#include "filterlist.h"
#include "ktfeed.h"
#include "managefiltersdlg.h"
#include "syndicationplugin.h"
#include <util/log.h>

using namespace bt;

namespace kt
{
KLocalizedString FeedWidget::item_template = ki18nc(
    /* Context */
    "Given string needs to be HTML parseable."
    "%1 is title of RSS feed item."
    "%2 is publishing date of RSS feed item."
    "%3 is description given in RSS feed item."
    "%4 is CSS Hex RGB formatted text color."
    "%5 is CSS Hex RGB formatted background color.",
    /* Template Text */
    "<html>\
    <body style=\"color:%4; background-color:%5\">\
    <div style=\"border-style:solid; border-width:1px; border-color:%4; margin:5px; padding:5px\">\
    <b>Title:</b> %1<br/>\
    <b>Date:</b> %2<br/>\
    </div>\
    <p>%3</p>\
    </body>\
    </html>\
    ");

FeedWidget::FeedWidget(FilterList *filters, SyndicationActivity *act, QWidget *parent)
    : QWidget(parent)
    , feed(nullptr)
    , filters(filters)
    , act(act)
{
    setupUi(this);
    m_splitter->setStretchFactor(0, 3);
    m_splitter->setStretchFactor(1, 1);

    connect(m_download, &QPushButton::clicked, this, &FeedWidget::downloadClicked);
    connect(m_refresh, &QPushButton::clicked, this, &FeedWidget::refreshClicked);
    connect(m_filters, &QPushButton::clicked, this, &FeedWidget::filtersClicked);
    connect(m_refresh_rate, &QSpinBox::valueChanged, this, &FeedWidget::refreshRateChanged);
    connect(m_cookies, &QPushButton::clicked, this, &FeedWidget::cookiesClicked);

    m_refresh->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    m_filters->setIcon(QIcon::fromTheme(QStringLiteral("view-filter")));
    m_cookies->setIcon(QIcon::fromTheme(QStringLiteral("preferences-web-browser-cookies")));
    m_download->setIcon(QIcon::fromTheme(QStringLiteral("ktorrent")));

    model = new FeedWidgetModel(this);
    m_item_list->setModel(model);
    m_item_list->setAlternatingRowColors(true);
    m_item_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QHeaderView *hv = m_item_list->header();
    hv->setSectionResizeMode(QHeaderView::Interactive);
    connect(m_item_list->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FeedWidget::selectionChanged);

    m_download->setEnabled(false);
    m_url->clear();
    m_refresh_rate->clear();
    m_active_filters->clear();

    // Having this empty html container enables background colour being set by theme
    m_item_view->setHtml(QStringLiteral("<html></html>"));
    m_item_view->setEnabled(false);
    // An easy dark mode for most links that might open
    m_item_view->settings()->setAttribute(QWebEngineSettings::ForceDarkMode, (QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark));

    setEnabled(false);
}

FeedWidget::~FeedWidget()
{
}

void FeedWidget::loadState(KConfigGroup &g)
{
    m_splitter->restoreState(g.readEntry("feed_widget_splitter", QByteArray()));

    QHeaderView *hv = m_item_list->header();
    QByteArray state = g.readEntry("feed_widget_list_header", QByteArray());
    if (!state.isEmpty())
        hv->restoreState(state);
    else
        QTimer::singleShot(3000, this, &FeedWidget::resizeColumns);
}

void FeedWidget::saveState(KConfigGroup &g)
{
    g.writeEntry("feed_widget_splitter", m_splitter->saveState());
    QHeaderView *hv = m_item_list->header();
    g.writeEntry("feed_widget_list_header", hv->saveState());
}

void FeedWidget::resizeColumns()
{
    m_item_list->header()->resizeSections(QHeaderView::ResizeToContents);
}

void FeedWidget::setFeed(Feed *f)
{
    if (feed) {
        disconnect(feed, &Feed::updated, this, &FeedWidget::updated);
        disconnect(feed, &Feed::feedRenamed, this, &FeedWidget::onFeedRenamed);
        feed = nullptr;
    }

    feed = f;
    setEnabled(feed != nullptr);
    model->setCurrentFeed(f);
    if (feed) {
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

    const QModelIndexList sel = m_item_list->selectionModel()->selectedRows();
    for (const QModelIndex &idx : sel) {
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
    if (dlg.exec() == QDialog::Accepted) {
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
    QString nc = QInputDialog::getText(nullptr, i18n("Authentication Cookie"), i18n("Enter the new authentication cookie"), QLineEdit::Normal, cookie, &ok);
    if (ok) {
        feed->setAuthenticationCookie(nc);
        feed->save();
    }
}

void FeedWidget::selectionChanged(const QItemSelection &sel, const QItemSelection &prev)
{
    Q_UNUSED(prev);
    m_download->setEnabled(sel.count() > 0);
    m_item_view->setEnabled(sel.count() > 0);
    if (sel.count() > 0 && feed) {
        Syndication::ItemPtr item = model->itemForIndex(m_item_list->selectionModel()->selectedRows().front());
        if (item) {
            m_item_view->setHtml(item_template.subs(item->title())
                                     .subs(QLocale().toString(QDateTime::fromSecsSinceEpoch(item->datePublished()), QLocale::ShortFormat))
                                     .subs(item->description())
                                     .subs(QApplication::palette().text().color().name(QColor::NameFormat::HexRgb))
                                     .subs(QApplication::palette().window().color().name(QColor::NameFormat::HexRgb))
                                     // Making sure KLocalizedString doesn't process any of the tags
                                     .toString(Kuit::PlainText),
                                 QUrl(feed->feedData()->link()));
        }
    }
}

void FeedWidget::updated()
{
    if (!feed)
        return;

    switch (feed->feedStatus()) {
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
    Q_EMIT updateCaption(this, feed->title());
    m_active_filters->setText(QStringLiteral("<b>") + feed->filterNamesString() + QStringLiteral("</b>"));
}

void FeedWidget::onFeedRenamed(kt::Feed *f)
{
    Q_EMIT updateCaption(this, f->displayName());
}
}

#include "moc_feedwidget.cpp"
