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


#include "magnetview.h"
#include "magnetmodel.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QBoxLayout>
#include <QIcon>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QTreeView>

#include <torrent/magnetmanager.h>

namespace kt
{

    MagnetView::MagnetView(MagnetManager *magnetManager, QWidget* parent)
        : QWidget(parent)
        , mman(magnetManager)
    {
        model = new MagnetModel(magnetManager, this);

        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setMargin(0);
        layout->setSpacing(0);

        // magnets view
        view = new QTreeView(this);
        view->setModel(model);
        view->setUniformRowHeights(true);
        view->setRootIsDecorated(false);
        view->setAlternatingRowColors(true);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSortingEnabled(false);
        view->setAllColumnsShowFocus(true);
        view->setSelectionMode(QAbstractItemView::ContiguousSelection);
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(view, &QTreeView::customContextMenuRequested, this, &MagnetView::showContextMenu);
        layout->addWidget(view);

        // context menu
        menu = new QMenu(this);
        start = menu->addAction(QIcon::fromTheme(QStringLiteral("kt-start")), i18n("Start Magnet"), this, SLOT(startMagnetDownload()));
        stop = menu->addAction(QIcon::fromTheme(QStringLiteral("kt-stop")), i18n("Stop Magnet"), this, SLOT(stopMagnetDownload()));
        menu->addSeparator();
        remove = menu->addAction(QIcon::fromTheme(QStringLiteral("kt-remove")), i18n("Remove Magnet"), this, SLOT(removeMagnetDownload()));
    }

    MagnetView::~MagnetView()
    {
    }

    void MagnetView::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("MagnetView");
        QByteArray s = QByteArray::fromBase64(g.readEntry("state", QByteArray()));
        if (!s.isEmpty())
        {
            QHeaderView* v = view->header();
            v->restoreState(s);
        }
    }

    void MagnetView::saveState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("MagnetView");
        g.writeEntry("state", view->header()->saveState().toBase64());
    }

    void MagnetView::showContextMenu(QPoint p)
    {
        const QModelIndexList idx_list = view->selectionModel()->selectedRows();

        start->setEnabled(false);
        stop->setEnabled(false);
        remove->setEnabled(idx_list.count() > 0);

        for (const QModelIndex& idx : idx_list)
        {
            if (!mman->isStopped(idx.row()))
                stop->setEnabled(true);
            else
                start->setEnabled(true);
        }
        menu->popup(view->viewport()->mapToGlobal(p));
    }

    void MagnetView::removeMagnetDownload()
    {
        QModelIndexList idx_list = view->selectionModel()->selectedRows();
        if (!idx_list.isEmpty())
            mman->removeMagnets(idx_list.front().row(), idx_list.size());
    }

    void MagnetView::startMagnetDownload()
    {
        QModelIndexList idx_list = view->selectionModel()->selectedRows();
        if (!idx_list.isEmpty()) {
            mman->start(idx_list.front().row(), idx_list.size());
            view->clearSelection();
        }
    }

    void MagnetView::stopMagnetDownload()
    {
        QModelIndexList idx_list = view->selectionModel()->selectedRows();
        if (!idx_list.isEmpty()) {
            mman->stop(idx_list.front().row(), idx_list.size());
            view->clearSelection();
        }
    }

    void MagnetView::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Delete)
        {
            removeMagnetDownload();
            event->accept();
        }
        else
            QWidget::keyPressEvent(event);
    }
}

