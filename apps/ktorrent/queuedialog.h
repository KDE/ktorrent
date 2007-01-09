/***************************************************************************
 *   Copyright (C) 2005 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#ifndef QUEUEDIALOG_H
#define QUEUEDIALOG_H

#include "queuedlg.h"
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>

#include <qlistview.h>
#include <qstring.h>

class QueueItem: public QListViewItem
{
	public:
		QueueItem(kt::TorrentInterface* t, QListView* parent);
		
		int getPriority() { return torrentPriority; }
		void setPriority(int p);
		int compare(QListViewItem *i, int col, bool ascending ) const;
		
		void setTorrentPriority(int p);
		
		const kt::TorrentInterface* getTC() { return tc; }
		
	private:
		//void updatePriorities(QueueItem* to, bool from_end, int val);
		void paintCell(QPainter* p,const QColorGroup & cg,int column,int width,int align);
		
		kt::TorrentInterface* tc;
		int torrentPriority;
};

class QueueDialog: public QueueDlg 
{
	Q_OBJECT
	public:
		QueueDialog(bt::QueueManager* qm, QWidget *parent = 0, const char *name = 0);
	public slots:
		virtual void btnMoveUp_clicked();
		virtual void btnClose_clicked();
		virtual void btnMoveDown_clicked();
   		virtual void btnDequeue_clicked();
    	virtual void btnEnqueue_clicked();
    	virtual void btnApply_clicked();
    	virtual void btnOk_clicked();
    	virtual void seedList_currentChanged(QListViewItem*);
    	virtual void downloadList_currentChanged(QListViewItem*);
    virtual void btnMoveBottom_clicked();
    virtual void btnMoveTop_clicked();
    	
		
	private:
		///Enqueue item curr
		void enqueue(QueueItem* curr = 0);
		
		///Writes the queue order into QueueManager
		void writeQueue();
		
		///Gets the pointer to currently visible torrentList (download or seed)
		QListView* getCurrentList();

		bt::QueueManager* qman;
};

#endif
