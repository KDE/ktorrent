/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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

#include "peerview.h"

#include <QHeaderView>
#include <QIcon>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <KConfigGroup>
#include <KLocalizedString>

#include <interfaces/peerinterface.h>
#include <peer/accessmanager.h>
#include <util/functions.h>
#include "peerviewmodel.h"

using namespace bt;

namespace kt
{


    PeerView::PeerView(QWidget* parent) : QTreeView(parent)
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
        context_menu->addAction(QIcon::fromTheme(QStringLiteral("list-remove-user")), i18n("Kick Peer"), this, SLOT(kickPeer()));
        context_menu->addAction(QIcon::fromTheme(QStringLiteral("view-filter")), i18n("Ban Peer"), this, SLOT(banPeer()));
        connect(this, &PeerView::customContextMenuRequested, this, &PeerView::showContextMenu);
    }

    PeerView::~PeerView()
    {
    }

    void PeerView::showContextMenu(const QPoint& pos)
    {
        if (selectionModel()->selectedRows().count() == 0)
            return;

        context_menu->popup(viewport()->mapToGlobal(pos));
    }

    void PeerView::banPeer()
    {
        AccessManager& aman = AccessManager::instance();

        QModelIndexList indices = selectionModel()->selectedRows();
        foreach (const QModelIndex& idx, indices)
        {
            bt::PeerInterface* peer = model->indexToPeer(pm->mapToSource(idx));
            if (peer)
            {
                aman.banPeer(peer->getStats().ip_address);
                peer->kill();
            }
        }
    }

    void PeerView::kickPeer()
    {
        QModelIndexList indices = selectionModel()->selectedRows();
        foreach (const QModelIndex& idx, indices)
        {
            bt::PeerInterface* peer = model->indexToPeer(pm->mapToSource(idx));
            if (peer)
                peer->kill();
        }
    }

    void PeerView::peerAdded(PeerInterface* peer)
    {
        model->peerAdded(peer);
    }

    void PeerView::peerRemoved(PeerInterface* peer)
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
        if (!s.isEmpty())
        {
            QHeaderView* v = header();
            v->restoreState(s);
            sortByColumn(v->sortIndicatorSection(), v->sortIndicatorOrder());
            pm->sort(v->sortIndicatorSection(), v->sortIndicatorOrder());
        }
    }
}

