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

#include "chunkdownloadview.h"

#include <QHeaderView>
#include <QSortFilterProxyModel>

#include <KConfigGroup>
#include <KLocalizedString>

#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/chunkdownloadinterface.h>
#include <util/functions.h>
#include <util/log.h>
#include "chunkdownloadmodel.h"

using namespace bt;

namespace kt
{


    ChunkDownloadView::ChunkDownloadView(QWidget* parent) : QWidget(parent)
    {
        setupUi(this);

        model = new ChunkDownloadModel(this);
        pm = new QSortFilterProxyModel(this);
        pm->setSourceModel(model);
        pm->setDynamicSortFilter(true);
        pm->setSortRole(Qt::UserRole);

        m_chunk_view->setModel(pm);
        m_chunk_view->setRootIsDecorated(false);
        m_chunk_view->setSortingEnabled(true);
        m_chunk_view->setAlternatingRowColors(true);
        m_chunk_view->setUniformRowHeights(true);

        QFont f = font();
        f.setBold(true);
        m_chunks_downloaded->setFont(f);
        m_chunks_downloading->setFont(f);
        m_chunks_left->setFont(f);
        m_excluded_chunks->setFont(f);
        m_size_chunks->setFont(f);
        m_total_chunks->setFont(f);
    }

    ChunkDownloadView::~ChunkDownloadView()
    {
    }

    void ChunkDownloadView::downloadAdded(ChunkDownloadInterface* cd)
    {
        model->downloadAdded(cd);
    }

    void ChunkDownloadView::downloadRemoved(ChunkDownloadInterface* cd)
    {
        model->downloadRemoved(cd);
    }

    void ChunkDownloadView::update()
    {
        if (!curr_tc)
            return;

        model->update();
        const TorrentStats& s = curr_tc.data()->getStats();
        m_chunks_downloading->setText(QString::number(s.num_chunks_downloading));
        m_chunks_downloaded->setText(QString::number(s.num_chunks_downloaded));
        m_excluded_chunks->setText(QString::number(s.num_chunks_excluded));
        m_chunks_left->setText(QString::number(s.num_chunks_left));
    }

    void ChunkDownloadView::changeTC(TorrentInterface* tc)
    {
        curr_tc = tc;
        if (!curr_tc)
        {
            setEnabled(false);
        }
        else
        {
            setEnabled(true);
            const TorrentStats& s = curr_tc.data()->getStats();
            m_total_chunks->setText(QString::number(s.total_chunks));
            m_size_chunks->setText(BytesToString(s.chunk_size));
        }
        model->changeTC(tc);
    }

    void ChunkDownloadView::removeAll()
    {
        model->clear();
    }

    void ChunkDownloadView::saveState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("ChunkDownloadView");
        QByteArray s = m_chunk_view->header()->saveState();
        g.writeEntry("state", s.toBase64());
    }

    void ChunkDownloadView::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("ChunkDownloadView");
        QByteArray s = QByteArray::fromBase64(g.readEntry("state", QByteArray()));
        if (!s.isEmpty())
        {
            QHeaderView* v = m_chunk_view->header();
            v->restoreState(s);
            m_chunk_view->sortByColumn(v->sortIndicatorSection(), v->sortIndicatorOrder());
            model->sort(v->sortIndicatorSection(), v->sortIndicatorOrder());
        }
    }
}

