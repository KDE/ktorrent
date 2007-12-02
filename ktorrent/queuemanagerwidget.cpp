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
#include <QHeaderView>
#include <klocale.h>
#include <kstandardguiitem.h>
#include <torrent/queuemanager.h>
#include "queuemanagerwidget.h"
#include "queuemanagermodel.h"

namespace kt
{

	QueueManagerWidget::QueueManagerWidget(QueueManager* qman,QWidget* parent) : QWidget(parent),qman(qman)
	{
		setupUi(this);
		
		connect(m_move_up,SIGNAL(clicked()),this,SLOT(moveUpClicked()));
		connect(m_move_down,SIGNAL(clicked()),this,SLOT(moveDownClicked()));
		connect(m_queue,SIGNAL(clicked()),this,SLOT(queueClicked()));
		
		
		m_move_up->setIcon(KIcon("go-up"));
		m_move_down->setIcon(KIcon("go-down"));
		m_queue->setIcon(KIcon("view-choose"));
		m_queue->setToolTip(i18n("Enqueue or dequeue a torrent"));
		
		model = new QueueManagerModel(qman,this);
		m_torrents->setModel(model);
		m_torrents->setRootIsDecorated(false);
		m_torrents->setAlternatingRowColors(true);
		m_torrents->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_torrents->setSortingEnabled(false);
		m_torrents->setDragDropMode(QAbstractItemView::InternalMove);
		m_torrents->setDragEnabled(true);
		m_torrents->setAcceptDrops(true);
		m_torrents->setDropIndicatorShown(true);
	}


	QueueManagerWidget::~QueueManagerWidget()
	{}
	
	void QueueManagerWidget::onTorrentAdded(bt::TorrentInterface* tc)
	{
		model->onTorrentAdded(tc);
	}
	
	void QueueManagerWidget::onTorrentRemoved(bt::TorrentInterface* tc)
	{
		model->onTorrentRemoved(tc);
	}

	void QueueManagerWidget::moveUpClicked()
	{
		QModelIndex cur = m_torrents->selectionModel()->currentIndex();
		if (cur.isValid())
		{
			int r = cur.row();
			model->moveUp(r);
			if (r > 0)
				r--;
			m_torrents->selectionModel()->setCurrentIndex(model->index(r,0),QItemSelectionModel::Select|QItemSelectionModel::Rows);
		}
	}
	
	void QueueManagerWidget::moveDownClicked()
	{
		QModelIndex cur = m_torrents->selectionModel()->currentIndex();
		if (cur.isValid())
		{
			int r = cur.row();
			model->moveDown(r);
			if (r < model->rowCount(QModelIndex()) - 1)
				r++;
			m_torrents->selectionModel()->setCurrentIndex(model->index(r,0),QItemSelectionModel::Select|QItemSelectionModel::Rows);
		}
	}
	
	void QueueManagerWidget::queueClicked()
	{
		QModelIndex cur = m_torrents->selectionModel()->currentIndex();
		if (cur.isValid())
		{
			model->queue(cur.row());
			m_torrents->selectionModel()->setCurrentIndex(cur,QItemSelectionModel::Select|QItemSelectionModel::Rows);
		}
	}
	
	void QueueManagerWidget::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("QueueManagerWidget");
		QByteArray s = m_torrents->header()->saveState();
		g.writeEntry("view_state",s.toBase64());
	}
	
	void QueueManagerWidget::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("QueueManagerWidget");
		QByteArray s = QByteArray::fromBase64(g.readEntry("view_state",QByteArray()));
		if (!s.isNull())
			m_torrents->header()->restoreState(s);
	}
}

#include "queuemanagerwidget.moc"