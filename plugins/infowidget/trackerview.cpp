/*
    SPDX-FileCopyrightText: 2006-2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2006-2009 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "trackerview.h"

#include <QClipboard>
#include <QHeaderView>
#include <QUrl>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

#include "addtrackersdialog.h"
#include "trackermodel.h"
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerinterface.h>
#include <interfaces/trackerslist.h>
#include <torrent/globals.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
TrackerView::TrackerView(QWidget *parent)
    : QWidget(parent)
    , header_state_loaded(false)
{
    setupUi(this);
    model = new TrackerModel(this);
    proxy_model = new QSortFilterProxyModel(this);
    proxy_model->setSortRole(Qt::UserRole);
    proxy_model->setSourceModel(model);
    m_tracker_list->setModel(proxy_model);
    m_tracker_list->setAllColumnsShowFocus(true);
    m_tracker_list->setRootIsDecorated(false);
    m_tracker_list->setAlternatingRowColors(true);
    m_tracker_list->setSortingEnabled(true);
    m_tracker_list->setUniformRowHeights(true);
    connect(m_add_tracker, &QPushButton::clicked, this, &TrackerView::addClicked);
    connect(m_remove_tracker, &QPushButton::clicked, this, &TrackerView::removeClicked);
    connect(m_change_tracker, &QPushButton::clicked, this, &TrackerView::changeClicked);
    connect(m_restore_defaults, &QPushButton::clicked, this, &TrackerView::restoreClicked);
    connect(m_tracker_list->selectionModel(), &QItemSelectionModel::currentChanged, this, &TrackerView::currentChanged);
    connect(m_scrape, &QPushButton::clicked, this, &TrackerView::scrapeClicked);

    m_add_tracker->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    m_remove_tracker->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    m_restore_defaults->setIcon(QIcon::fromTheme(QLatin1String("kt-restore-defaults")));
    m_change_tracker->setIcon(QIcon::fromTheme(QLatin1String("kt-change-tracker")));

    m_ContextMenu = new QMenu(this);
    QAction *copy_URL = m_ContextMenu->addAction(i18n("Copy Tracker URL"));
    connect(copy_URL, &QAction::triggered, [=]() {
        bt::TrackerInterface *trk = selectedTracker();
        if (trk)
            QApplication::clipboard()->setText(trk->trackerURL().toDisplayString());
    });

    QAction *copy_status = m_ContextMenu->addAction(i18n("Copy Tracker status"));
    connect(copy_status, &QAction::triggered, [=]() {
        bt::TrackerInterface *trk = selectedTracker();
        if (trk)
            QApplication::clipboard()->setText(trk->trackerStatusString());
    });

    m_tracker_list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tracker_list, &QTreeView::customContextMenuRequested, [=](const QPoint &point) {
        QModelIndex index = m_tracker_list->indexAt(point);
        if (index.isValid()) {
            m_ContextMenu->exec(m_tracker_list->viewport()->mapToGlobal(point));
        }
    });

    setEnabled(false);
    torrentChanged(0);
}

TrackerView::~TrackerView()
{
}

void TrackerView::addClicked()
{
    if (!tc)
        return;

    AddTrackersDialog dlg(this, tracker_hints);
    if (dlg.exec() != QDialog::Accepted)
        return;

    const QStringList trackers = dlg.trackerList();
    QList<QUrl> urls;
    QStringList invalid;
    // check for invalid urls
    for (const QString &t : trackers) {
        if (t.isEmpty())
            continue;

        QUrl url(t.trimmed());
        if (!url.isValid() || (url.scheme() != QLatin1String("udp") && url.scheme() != QLatin1String("http") && url.scheme() != QLatin1String("https")))
            invalid.append(t);
        else {
            if (!tracker_hints.contains(url.toDisplayString()))
                tracker_hints.append(url.toDisplayString());
            urls.append(url);
        }
    }

    if (!invalid.isEmpty()) {
        KMessageBox::errorList(this, i18n("Several URL's could not be added because they are malformed:"), invalid);
    }

    QList<QUrl> dupes;
    QList<bt::TrackerInterface *> tl;
    for (const QUrl &url : qAsConst(urls)) {
        bt::TrackerInterface *trk = tc.data()->getTrackersList()->addTracker(url, true);
        if (!trk)
            dupes.append(url);
        else
            tl.append(trk);
    }

    if (dupes.size() == 1)
        KMessageBox::sorry(0, i18n("There already is a tracker named <b>%1</b>.", dupes.front().toDisplayString()));
    else if (dupes.size() > 1)
        KMessageBox::informationList(0, i18n("The following duplicate trackers were not added:"), QUrl::toStringList(dupes));

    if (!tl.isEmpty())
        model->addTrackers(tl);
}

void TrackerView::removeClicked()
{
    QModelIndex current = proxy_model->mapToSource(m_tracker_list->selectionModel()->currentIndex());
    if (!current.isValid())
        return;

    model->removeRow(current.row());
}

bt::TrackerInterface *TrackerView::selectedTracker() const
{
    QModelIndex current = m_tracker_list->selectionModel()->currentIndex();
    if (!current.isValid() || tc.isNull())
        return nullptr;

    return model->tracker(proxy_model->mapToSource(current));
}

void TrackerView::changeClicked()
{
    bt::TrackerInterface *trk = selectedTracker();
    if (trk && trk->isEnabled()) {
        bt::TrackersList *tlist = tc.data()->getTrackersList();
        tlist->setCurrentTracker(trk);
    }
}

void TrackerView::restoreClicked()
{
    if (tc) {
        tc.data()->getTrackersList()->restoreDefault();
        tc.data()->updateTracker();
        model->changeTC(tc.data()); // trigger reset
    }
}

void TrackerView::updateClicked()
{
    if (!tc)
        return;

    tc.data()->updateTracker();
}

void TrackerView::scrapeClicked()
{
    if (!tc)
        return;

    tc.data()->scrapeTracker();
}

void TrackerView::changeTC(TorrentInterface *ti)
{
    if (tc.data() == ti)
        return;

    setEnabled(ti != 0);
    torrentChanged(ti);
    update();

    if (!header_state_loaded) {
        m_tracker_list->resizeColumnToContents(0);
        header_state_loaded = true;
    }
}

void TrackerView::update()
{
    if (tc)
        model->update();
}

void TrackerView::torrentChanged(TorrentInterface *ti)
{
    tc = ti;
    if (!tc) {
        m_add_tracker->setEnabled(false);
        m_remove_tracker->setEnabled(false);
        m_restore_defaults->setEnabled(false);
        m_change_tracker->setEnabled(false);
        m_scrape->setEnabled(false);
        model->changeTC(0);
    } else {
        m_add_tracker->setEnabled(true);
        m_remove_tracker->setEnabled(true);
        m_restore_defaults->setEnabled(true);
        m_scrape->setEnabled(true);
        model->changeTC(ti);
        currentChanged(m_tracker_list->selectionModel()->currentIndex(), QModelIndex());
        m_tracker_list->resizeColumnToContents(0);
    }
}

void TrackerView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (!tc) {
        m_change_tracker->setEnabled(false);
        m_remove_tracker->setEnabled(false);
        return;
    }

    const TorrentStats &s = tc.data()->getStats();

    bt::TrackerInterface *trk = model->tracker(proxy_model->mapToSource(current));
    bool enabled = trk ? trk->isEnabled() : false;
    m_change_tracker->setEnabled(s.running && model->rowCount(QModelIndex()) > 1 && enabled && s.priv_torrent);
    m_remove_tracker->setEnabled(trk && tc.data()->getTrackersList()->canRemoveTracker(trk));
}

void TrackerView::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("TrackerView");
    QByteArray s = m_tracker_list->header()->saveState();
    g.writeEntry("state", s.toBase64());
    g.writeEntry("tracker_hints", tracker_hints);
}

void TrackerView::loadState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("TrackerView");
    QByteArray s = g.readEntry("state", QByteArray());
    if (!s.isEmpty()) {
        QHeaderView *v = m_tracker_list->header();
        v->restoreState(QByteArray::fromBase64(s));
        header_state_loaded = true;
    }

    QStringList default_hints;
    default_hints << QStringLiteral("udp://tracker.publicbt.com:80/announce") << QStringLiteral("udp://tracker.openbittorrent.com:80/announce");
    tracker_hints = g.readEntry("tracker_hints", default_hints);
}
}
