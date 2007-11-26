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
#include <kstandardguiitem.h>
#include <torrent/queuemanager.h>
#include "queuemanagerwidget.h"
#include "queuemanagermodel.h"

namespace kt
{

	QueueManagerWidget::QueueManagerWidget(QueueManager* qman,QWidget* parent) : QWidget(parent),qman(qman)
	{
		setupUi(this);
		
		connect(m_downloads_move_up,SIGNAL(clicked()),this,SLOT(moveDownloadUpClicked()));
		connect(m_downloads_move_down,SIGNAL(clicked()),this,SLOT(moveDownloadDownClicked()));
		connect(m_uploads_move_up,SIGNAL(clicked()),this,SLOT(moveUploadUpClicked()));
		connect(m_uploads_move_down,SIGNAL(clicked()),this,SLOT(moveUploadDownClicked()));
		
		m_downloads_move_up->setIcon(KIcon("go-up"));
		m_downloads_move_down->setIcon(KIcon("go-down"));
		m_uploads_move_up->setIcon(KIcon("go-up"));
		m_uploads_move_down->setIcon(KIcon("go-down"));
		
		downloads = new QueueManagerModel(QueueManagerModel::DOWNLOADS,qman,this);
		m_downloads->setModel(downloads);
		m_downloads->setRootIsDecorated(false);
		m_downloads->setAlternatingRowColors(true);
		m_downloads->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_downloads->setSortingEnabled(false);
		
		uploads = new QueueManagerModel(QueueManagerModel::UPLOADS,qman,this);
		m_uploads->setModel(uploads);
		m_uploads->setRootIsDecorated(false);
		m_uploads->setAlternatingRowColors(true);
		m_uploads->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_uploads->setSortingEnabled(false);
	}


	QueueManagerWidget::~QueueManagerWidget()
	{}
	
	void QueueManagerWidget::onTorrentAdded(bt::TorrentInterface* tc)
	{
		downloads->onTorrentAdded(tc);
		uploads->onTorrentAdded(tc);
	}
	
	void QueueManagerWidget::onTorrentRemoved(bt::TorrentInterface* tc)
	{
		downloads->onTorrentRemoved(tc);
		uploads->onTorrentRemoved(tc);
	}

	void QueueManagerWidget::moveDownloadUpClicked()
	{
		QModelIndex cur = m_downloads->selectionModel()->currentIndex();
		if (cur.isValid())
			downloads->moveUp(cur.row());
	}
	
	void QueueManagerWidget::moveDownloadDownClicked()
	{
		QModelIndex cur = m_downloads->selectionModel()->currentIndex();
		if (cur.isValid())
			downloads->moveDown(cur.row());
	}
	
	void QueueManagerWidget::moveUploadUpClicked()
	{
		QModelIndex cur = m_uploads->selectionModel()->currentIndex();
		if (cur.isValid())
			uploads->moveUp(cur.row());
	}
	
	void QueueManagerWidget::moveUploadDownClicked()
	{
		QModelIndex cur = m_uploads->selectionModel()->currentIndex();
		if (cur.isValid())
			uploads->moveDown(cur.row());
	}
	
	void QueueManagerWidget::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("QueueManagerWidget");
		QByteArray s = m_downloads->header()->saveState();
		g.writeEntry("downloads_state",s.toBase64());
		s = m_uploads->header()->saveState();
		g.writeEntry("uploads_state",s.toBase64());
	}
	
	void QueueManagerWidget::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("QueueManagerWidget");
		QByteArray s = QByteArray::fromBase64(g.readEntry("downloads_state",QByteArray()));
		if (!s.isNull())
			m_downloads->header()->restoreState(s);
		
		s = QByteArray::fromBase64(g.readEntry("uploads_state",QByteArray()));
		if (!s.isNull())
			m_uploads->header()->restoreState(s);
	}
}

#include "queuemanagerwidget.moc"