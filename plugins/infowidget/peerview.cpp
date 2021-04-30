/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "peerview.h"

#include <QHeaderView>
#include <QIcon>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <KConfigGroup>
#include <KLocalizedString>

#include "peerviewmodel.h"
#include <interfaces/peerinterface.h>
#include <peer/accessmanager.h>
#include <util/functions.h>

using namespace bt;

namespace kt
{
PeerView::PeerView(QWidget *parent)
    : QTreeView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);

    pm = new QSortFilterProxyModel(this);
    pm->setSortRole(Qt::UserRole);
    pm->setDynamicSortFilter(true);
    model = new PeerViewModel(this);
    pm->setSourceModel(model);
    setModel(pm);

    context_menu = new QMenu(this);
    context_menu->addAction(QIcon::fromTheme(QStringLiteral("list-remove-user")), i18n("Kick Peer"), this, &PeerView::kickPeer);
    context_menu->addAction(QIcon::fromTheme(QStringLiteral("view-filter")), i18n("Ban Peer"), this, &PeerView::banPeer);
    connect(this, &PeerView::customContextMenuRequested, this, &PeerView::showContextMenu);
}

PeerView::~PeerView()
{
}

void PeerView::showContextMenu(const QPoint &pos)
{
    if (selectionModel()->selectedRows().count() == 0)
        return;

    context_menu->popup(viewport()->mapToGlobal(pos));
}

void PeerView::banPeer()
{
    AccessManager &aman = AccessManager::instance();

    const QModelIndexList indices = selectionModel()->selectedRows();
    for (const QModelIndex &idx : indices) {
        bt::PeerInterface *peer = model->indexToPeer(pm->mapToSource(idx));
        if (peer) {
            aman.banPeer(peer->getStats().ip_address);
            peer->kill();
        }
    }
}

void PeerView::kickPeer()
{
    const QModelIndexList indices = selectionModel()->selectedRows();
    for (const QModelIndex &idx : indices) {
        bt::PeerInterface *peer = model->indexToPeer(pm->mapToSource(idx));
        if (peer)
            peer->kill();
    }
}

void PeerView::peerAdded(PeerInterface *peer)
{
    model->peerAdded(peer);
}

void PeerView::peerRemoved(PeerInterface *peer)
{
    model->peerRemoved(peer);
}

void PeerView::update()
{
    model->update();
}

void PeerView::removeAll()
{
    model->clear();
}

void PeerView::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("PeerView");
    QByteArray s = header()->saveState();
    g.writeEntry("state", s.toBase64());
}

void PeerView::loadState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("PeerView");
    QByteArray s = QByteArray::fromBase64(g.readEntry("state", QByteArray()));
    if (!s.isEmpty()) {
        QHeaderView *v = header();
        v->restoreState(s);
        sortByColumn(v->sortIndicatorSection(), v->sortIndicatorOrder());
        pm->sort(v->sortIndicatorSection(), v->sortIndicatorOrder());
    }
}
}
