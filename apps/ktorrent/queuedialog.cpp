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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "queuedialog.h"
#include <interfaces/torrentinterface.h>
#include <interfaces/functions.h>
#include <torrent/queuemanager.h>

#include <qlistview.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qptrlist.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <klocale.h>
#include <kglobal.h>
#include <kurl.h>
#include <kiconloader.h>
#include <ksqueezedtextlabel.h>
#include <ktabwidget.h>
#include <kpushbutton.h>
#include <kiconloader.h>

using namespace bt;
using namespace kt;

QueueItem::QueueItem(kt::TorrentInterface* t, QListView* parent)
	:QListViewItem(parent), tc(t)
{
	setPriority(tc->getPriority());
	setText(0, QString(tc->getStats().torrent_name));
}

int QueueItem::compare(QListViewItem *i, int , bool ) const
{
	QueueItem* it = (QueueItem*) i;
	if(it->getPriority() == torrentPriority)
	{
		const TorrentInterface* ti = it->getTC();
		QString name1 = tc->getStats().torrent_name;
		QString name2 = ti->getStats().torrent_name;
		return name1.compare(name2);
	}
			
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
	KIconLoader* iload = KGlobal::iconLoader();
	
	m_tabs->setTabIconSet(m_tabs->page(0), iload->loadIconSet("down", KIcon::Small));
	m_tabs->setTabIconSet(m_tabs->page(1), iload->loadIconSet("up", KIcon::Small));
	
 	logo->setPixmap(iload->loadIcon("ktqueuemanager", KIcon::Desktop));
	
	connect(downloadList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(downloadList_currentChanged( QListViewItem* )));
	connect(seedList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(seedList_currentChanged( QListViewItem* )));
	
	if(downloadList->firstChild())
		downloadList->setCurrentItem(downloadList->firstChild());
	
	if(seedList->firstChild())
		seedList->setCurrentItem(seedList->firstChild());
	
	btnMoveUp->setPixmap(iload->loadIcon("up", KIcon::Small));
	btnMoveDown->setPixmap(iload->loadIcon("down", KIcon::Small));
	
	this->qman = qm;

	QPtrList<kt::TorrentInterface>::iterator it = qman->begin();
	for( ; it != qman->end(); ++it)
	{
		TorrentInterface* tc = *it;
		TorrentStatus ts = tc->getStats().status;
		
		if(ts == kt::SEEDING || ts == kt::DOWNLOAD_COMPLETE || 
			ts == kt::SEEDING_COMPLETE || tc->getStats().completed)
		{
			QueueItem* item = new QueueItem(tc, seedList);
			seedList->insertItem(item);
		}
		else
		{
			QueueItem* item = new QueueItem(tc, downloadList);
			downloadList->insertItem(item);
		}
	}
}

void QueueDialog::btnMoveUp_clicked()
{	
	QueueItem* current = (QueueItem*) getCurrentList()->selectedItem();
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
		getCurrentList()->sort();
	}
}

void QueueDialog::btnMoveDown_clicked()
{
	QueueItem* current = (QueueItem*) getCurrentList()->selectedItem();
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
		getCurrentList()->sort();
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
	QueueItem* current = (QueueItem*) getCurrentList()->selectedItem();
	if(current == 0)
		return;
	if(current->getPriority() == 0)
		return;
			
	current->setPriority(0);
	getCurrentList()->sort();
}

void QueueDialog::enqueue(QueueItem* curr)
{
	QueueItem* current = curr == 0 ? (QueueItem*) getCurrentList()->selectedItem() : curr;
	if(current == 0)
		return;
	if(current->getPriority() != 0)
		return;
	
	QueueItem* item = (QueueItem*) getCurrentList()->firstChild();
	if(item == 0)
		return;
	
	while(item != 0)
	{
		if(item->getPriority() != 0)
			item->setPriority(item->getPriority() + 1);
		item = (QueueItem*) item->itemBelow();
	}
	
	current->setPriority(1);
	getCurrentList()->sort();
}

void QueueDialog::writeQueue()
{
	downloadList->sort();
	seedList->sort();
	
	int p = 0;
	
	QueueItem* item = (QueueItem*) downloadList->lastItem();
	if(item != 0)
	{
		while(item != 0)
		{
			if(item->getPriority() != 0)
				item->setTorrentPriority(++p);
			else
				item->setTorrentPriority(0);
			item = (QueueItem*) item->itemAbove();
		}
	}
	
	item = (QueueItem*) seedList->lastItem();
	if(item == 0)
	{
		qman->orderQueue();
		return;
	}
	
	p = 0;
	
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

QListView* QueueDialog::getCurrentList()
{
	return m_tabs->currentPageIndex() == 0 ? downloadList : seedList;
}

void QueueDialog::downloadList_currentChanged(QListViewItem* item)
{
	if(!item)
	{
		dlStatus->clear();
		dlTracker->clear();
		dlRatio->clear();
		dlDHT->clear();
		return;
	}
	
	const TorrentInterface* tc = ((QueueItem*)item)->getTC();
	TorrentStats s = tc->getStats();
	
	dlStatus->setText(tc->statusToString());
	dlTracker->setText(tc->getTrackersList()->getTrackerURL().prettyURL());
	dlRatio->setText(QString("%1").arg((float)s.bytes_uploaded / s.bytes_downloaded,0,'f',2));
	dlBytes->setText(BytesToString(s.bytes_left_to_download));
	dlDHT->setText(s.priv_torrent ? i18n("No (private torrent)") : i18n("Yes"));
}

void QueueDialog::seedList_currentChanged(QListViewItem* item)
{
	if(!item)
	{
		ulStatus->clear();
		ulTracker->clear();
		ulRatio->clear();
		ulDHT->clear();
		return;
	}
	
	const TorrentInterface* tc = ((QueueItem*)item)->getTC();
	TorrentStats s = tc->getStats();
	
	ulStatus->setText(tc->statusToString());
	ulTracker->setText(tc->getTrackersList()->getTrackerURL().prettyURL());
	ulRatio->setText(QString("%1").arg((float)s.bytes_uploaded / s.bytes_downloaded,0,'f',2));
	ulBytes->setText(BytesToString(s.bytes_uploaded));
	ulDHT->setText(s.priv_torrent ? i18n("No (private torrent)") : i18n("Yes"));
}

void QueueDialog::btnMoveTop_clicked()
{
	QueueItem* current = (QueueItem*) getCurrentList()->selectedItem();
	if(current == 0)
		return;
	
	if(current->getPriority() == 0)
		return;
	
	QueueItem* previous = (QueueItem*) current->itemAbove();
	
	if(previous == 0)
		return;	
	
	int p = previous->getPriority();
	
	while(previous != 0)
	{
		p = previous->getPriority();
		previous->setPriority(p - 1);
		
		previous = (QueueItem*) previous->itemAbove();
	}
	
	current->setPriority(p);
	getCurrentList()->sort();
}

void QueueDialog::btnMoveBottom_clicked()
{
	QueueItem* current = (QueueItem*) getCurrentList()->selectedItem();
	if(current == 0)
		return;
	
	if(current->getPriority() == 0)
		return;
	
	QueueItem* previous = (QueueItem*) current->itemBelow();
	
	if(previous == 0)
		return;	
	
	if(previous->getPriority() == 0)
		return;
	
	int p = previous->getPriority();
	
	while(previous != 0 && previous->getPriority() != 0)
	{
		p = previous->getPriority();
		previous->setPriority(p + 1);
		
		previous = (QueueItem*) previous->itemBelow();
	}
	
	current->setPriority(p);
	getCurrentList()->sort();
}



#include "queuedialog.moc"

