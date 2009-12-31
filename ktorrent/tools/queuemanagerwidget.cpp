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
		connect(m_move_top,SIGNAL(clicked()),this,SLOT(moveTopClicked()));
		connect(m_move_bottom,SIGNAL(clicked()),this,SLOT(moveBottomClicked()));
		
		m_move_up->setIcon(KIcon("go-up"));
		m_move_up->setToolTip(i18n("Move a torrent up in the queue"));
		m_move_down->setIcon(KIcon("go-down"));
		m_move_down->setToolTip(i18n("Move a torrent down in the queue"));
		m_move_top->setIcon(KIcon("go-top"));
		m_move_top->setToolTip(i18n("Move a torrent to the top of the queue"));
		m_move_bottom->setIcon(KIcon("go-bottom"));
		m_move_bottom->setToolTip(i18n("Move a torrent to the bottom of the queue"));
		
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
		
		m_torrents->setSelectionMode(QAbstractItemView::ContiguousSelection);
		
		connect(m_search,SIGNAL(textChanged(QString)),this,SLOT(searchTextChanged(QString)));
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
		QModelIndexList sel = m_torrents->selectionModel()->selectedRows();
		QList<int> rows;
		foreach (const QModelIndex & idx,sel)
			rows.append(idx.row());
		
		if (rows.front() == 0)
			return;
		
		model->moveUp(rows.front(),rows.count());
		
		QItemSelection nsel;
		int cols = model->columnCount(QModelIndex());
		nsel.select(model->index(rows.front() - 1,0),model->index(rows.back() - 1,cols - 1));
		m_torrents->selectionModel()->select(nsel,QItemSelectionModel::Select);
	}
	
	void QueueManagerWidget::moveDownClicked()
	{
		QModelIndexList sel = m_torrents->selectionModel()->selectedRows();
		QList<int> rows;
		foreach (const QModelIndex & idx,sel)
			rows.append(idx.row());
		
		int rowcount = model->rowCount(QModelIndex());
		if (rows.back() == rowcount - 1)
			return;
		
		model->moveDown(rows.front(),rows.count());
		
		QItemSelection nsel;
		int cols = model->columnCount(QModelIndex());
		nsel.select(model->index(rows.front() + 1,0),model->index(rows.back() + 1,cols - 1));
		m_torrents->selectionModel()->select(nsel,QItemSelectionModel::Select);
	}
	
	void QueueManagerWidget::moveTopClicked()
	{
		QModelIndexList sel = m_torrents->selectionModel()->selectedRows();
		QList<int> rows;
		foreach (const QModelIndex & idx,sel)
			rows.append(idx.row());
		
		if (rows.front() == 0)
			return;
		
		model->moveTop(rows.front(),rows.count());
		
		QItemSelection nsel;
		int cols = model->columnCount(QModelIndex());
		nsel.select(model->index(0,0),model->index(rows.count() - 1,cols - 1));
		m_torrents->selectionModel()->select(nsel,QItemSelectionModel::Select);
		m_torrents->scrollToTop();
	}
	
	void QueueManagerWidget::moveBottomClicked()
	{
		QModelIndexList sel = m_torrents->selectionModel()->selectedRows();
		QList<int> rows;
		foreach (const QModelIndex & idx,sel)
			rows.append(idx.row());
		
		int rowcount = model->rowCount(QModelIndex());
		if (rows.back() == rowcount - 1)
			return;
		
		model->moveBottom(rows.front(),rows.count());
		
		QItemSelection nsel;
		int cols = model->columnCount(QModelIndex());
		nsel.select(model->index(rowcount - rows.count(),0),model->index(rowcount - 1,cols - 1));
		m_torrents->selectionModel()->select(nsel,QItemSelectionModel::Select);
		m_torrents->scrollToBottom();
	}
	
	void QueueManagerWidget::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("QueueManagerWidget");
		QByteArray s = m_torrents->header()->saveState();
		g.writeEntry("view_state",s.toBase64());
		g.writeEntry("search_text",m_search->text());
	}
	
	void QueueManagerWidget::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("QueueManagerWidget");
		QByteArray s = QByteArray::fromBase64(g.readEntry("view_state",QByteArray()));
		if (!s.isNull())
			m_torrents->header()->restoreState(s);
		
		QString st = g.readEntry("search_text",QString());
		if (!st.isEmpty())
			m_search->setText(st);
	}
	
	void QueueManagerWidget::update()
	{
		model->update();
	}
	
	void QueueManagerWidget::searchTextChanged(const QString& t)
	{
		QModelIndex idx = model->find(t);
		if (idx.isValid())
		{
			m_torrents->scrollTo(idx,QAbstractItemView::PositionAtCenter);
		}
	}

}

#include "queuemanagerwidget.moc"
