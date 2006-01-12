/***************************************************************************
 *   Copyright (C) 2005 by Ivan Vasić   *
 *   ivasic@gmail.com   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "queuedialog.h"
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>

#include <qlistview.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qptrlist.h>

#include <klocale.h>

using namespace bt;
using namespace kt;

QueueItem::QueueItem(kt::TorrentInterface* t, QListView* parent)
	:QListViewItem(parent), tc(t)
{
	setPriority(tc->getPriority());
	setText(0, QString(tc->getStats().torrent_name));
}

int QueueItem::compare(QListViewItem *i, int col, bool ascending ) const
{
	QueueItem* it = (QueueItem*) i;
	if(it->getPriority() == torrentPriority)
		return 0;
			
	return it->getPriority() < torrentPriority ? -1 : 1;
}

void QueueItem::setPriority(int p) 
{ 
	torrentPriority = p;

	if(p==0)
		setText(1, i18n("User"));
	else
		setText(1, i18n("Queue Manager"));
}

void QueueItem::setTorrentPriority(int p)
{
	tc->setPriority(p);
}

void QueueItem::paintCell(QPainter* p,const QColorGroup & cg,int column,int width,int align)
{
	QColorGroup colorGrp( cg );
	QColor txt = colorGrp.text();

	//if (column == 1)
	if(torrentPriority == 0)
		colorGrp.setColor(QColorGroup::Text, Qt::gray);
	else
		colorGrp.setColor(QColorGroup::Text, txt);


	QListViewItem::paintCell(p,colorGrp,column,width,align);
}

QueueDialog::QueueDialog(bt::QueueManager* qm, QWidget *parent, const char *name)
	:QueueDlg(parent, name)
{
	this->qman = qm;
	
	torrentView = torrentList;

	QPtrList<kt::TorrentInterface>::iterator it = qman->begin();
	for( ; it != qman->end(); ++it)
	{
		TorrentInterface* tc = *it;
		TorrentStatus ts = tc->getStats().status;
		if(ts == kt::SEEDING || ts == kt::COMPLETE || tc->getStats().completed)
			continue;
		QueueItem* item = new QueueItem(tc, torrentView);
		torrentView->insertItem(item);
	}
}

void QueueDialog::btnMoveUp_clicked()
{	
	QueueItem* current = (QueueItem*) torrentView->selectedItem();
	if(current == 0)
		return;
	
	if(current->getPriority() == 0)
		return;
	
	QueueItem* previous = (QueueItem*) current->itemAbove();
	if(previous == 0)
		return;
	else
	{
		int tmp = previous->getPriority();
		previous->setPriority(current->getPriority());
		current->setPriority(tmp);
		torrentView->sort();
	}
}

void QueueDialog::btnMoveDown_clicked()
{	
	QueueItem* current = (QueueItem*) torrentView->selectedItem();
	if(current == 0)
		return;
	
	if(current->getPriority() == 0)
		return;
	
	QueueItem* previous = (QueueItem*) current->itemBelow();
	if(previous == 0)
		return;
	else
	{
		int tmp = previous->getPriority();
		if(tmp == 0)
			return;
		previous->setPriority(current->getPriority());
		current->setPriority(tmp);
		torrentView->sort();
	}
}

void QueueDialog::btnClose_clicked()
{
	this->close();
}

void QueueDialog::btnEnqueue_clicked()
{
	enqueue();
}

void QueueDialog::btnDequeue_clicked()
{
	QueueItem* current = (QueueItem*) torrentView->selectedItem();
	if(current == 0)
		return;
	if(current->getPriority() == 0)
		return;
			
	current->setPriority(0);
	torrentView->sort();
}

void QueueDialog::enqueue(QueueItem* curr)
{
	QueueItem* current = curr == 0 ? (QueueItem*) torrentView->selectedItem() : curr;
	if(current == 0)
		return;
	if(current->getPriority() != 0)
		return;
	
	QueueItem* item = (QueueItem*) torrentView->firstChild();
	if(item == 0)
		return;
	
	while(item != 0)
	{
		if(item->getPriority() != 0)
			item->setPriority(item->getPriority() + 1);
		item = (QueueItem*) item->itemBelow();
	}
	
	current->setPriority(1);
	torrentView->sort();
}

void QueueDialog::writeQueue()
{
	QueueItem* item = (QueueItem*) torrentView->lastItem();
	if(item == 0)
		return;
	
	int p = 0;
	
	while(item != 0)
	{
		if(item->getPriority() != 0)
			item->setTorrentPriority(++p);
		else
			item->setTorrentPriority(0);
		item = (QueueItem*) item->itemAbove();
	}
	qman->orderQueue();
}

void QueueDialog::btnApply_clicked()
{
	writeQueue();
}

void QueueDialog::btnOk_clicked()
{
	writeQueue();
	this->close();
}



