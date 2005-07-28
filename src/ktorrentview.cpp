/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <libtorrent/torrentcontrol.h>
#include <libtorrent/globals.h>
#include <kmessagebox.h>
#include "ktorrentview.h"
#include "ktorrentviewitem.h"
#include "debugview.h"



using namespace bt;

KTorrentView::KTorrentView(QWidget *parent)
	: KListView(parent),show_debug_view(false),menu(0),curr(0)

{
	connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
	addColumn(i18n("File"));
	addColumn(i18n("Status"));
	addColumn(i18n("Downloaded"));
	addColumn(i18n("Uploaded"));
	addColumn(i18n("Down Speed"));
	addColumn(i18n("Up Speed"));
	addColumn(i18n("Time Left"));
	addColumn(i18n("Peers"));
	addColumn(i18n("% Complete"));
	timer.start(1000);
	connect(this,SIGNAL(clicked(QListViewItem* )),this,SLOT(onExecuted(QListViewItem* )));
	show_debug_view = bt::Globals::instance().isDebugModeSet();
	connect(this,SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint& )),
			this,SLOT(showContextMenu(KListView*, QListViewItem*, const QPoint& )));

	KIconLoader* iload = KGlobal::iconLoader();
	menu = new KPopupMenu(this);
	
	stop_id = menu->insertItem(
			iload->loadIconSet("player_stop",KIcon::Small),i18n("Stop"),
			this,SLOT(stopDownload()));

	start_id = menu->insertItem(
			iload->loadIconSet("player_play",KIcon::Small),i18n("Start"),
			this,SLOT(startDownload()));

	remove_id = menu->insertItem(
			iload->loadIconSet("remove",KIcon::Small),i18n("Remove"),
			this,SLOT(removeDownload()));
	menu->insertSeparator();

	menu->insertItem(i18n("Manual announce"),this,SLOT(manualAnnounce()));

	setAllColumnsShowFocus(true);

	setColumnWidth(0,200);
	setColumnWidthMode(0,QListView::Manual);
	setShowSortIndicator(true);
}

KTorrentView::~KTorrentView()
{}

void KTorrentView::setShowDebugView(bool yes)
{
	show_debug_view = yes;
}

void KTorrentView::startDownload()
{
	if (!curr)
		return;

	bt::TorrentControl* tc = curr->getTC();
	tc->start();
}
	
void KTorrentView::stopDownload()
{
	if (!curr)
		return;

	bt::TorrentControl* tc = curr->getTC();
	tc->stop();
}
	
void KTorrentView::removeDownload()
{
	if (!curr)
		return;

	bt::TorrentControl* tc = curr->getTC();
	wantToRemove(tc);
}

void KTorrentView::manualAnnounce()
{
	if (!curr)
		return;

	bt::TorrentControl* tc = curr->getTC();
	tc->updateTracker();
}
		
bt::TorrentControl* KTorrentView::getCurrentTC()
{
	KTorrentViewItem* tvi = dynamic_cast<KTorrentViewItem*>(currentItem());
	if (tvi)
		return tvi->getTC();
	else
		return 0;
}

void KTorrentView::onExecuted(QListViewItem* item)
{
	KTorrentViewItem* tvi = dynamic_cast<KTorrentViewItem*>(item);
	if (tvi)
	{
		torrentClicked(tvi->getTC());
		currentChanged(tvi->getTC());
	}
}

void KTorrentView::showContextMenu(KListView* ,QListViewItem* item,const QPoint & p)
{
	if (!item)
		return;

	curr = dynamic_cast<KTorrentViewItem*>(item);
	if (curr)
	{
		bt::TorrentControl* tc = curr->getTC();
		menu->setItemEnabled(start_id,!tc->isRunning());
		menu->setItemEnabled(stop_id,tc->isRunning());
		menu->setItemEnabled(remove_id,true);
		menu->popup(p);
	}
}

void KTorrentView::addTorrent(bt::TorrentControl* tc)
{
	KTorrentViewItem* tvi = new KTorrentViewItem(this,tc);
	items.insert(tc,tvi);
	tvi->update();
	currentChanged(tc);
	connect(tc,SIGNAL(trackerError(bt::TorrentControl*,const QString & )),
			this,SLOT(onTrackerError(bt::TorrentControl*,const QString & )));
	if (show_debug_view)
	{
	//	DebugView* dbg = new DebugView(tc);
	//	dbg->show();
	}
}

void KTorrentView::removeTorrent(bt::TorrentControl* tc)
{
	QMap<bt::TorrentControl*,KTorrentViewItem*>::iterator i = items.find(tc);
	if (i != items.end())
	{
		KTorrentViewItem* tvi = i.data();
		items.erase(i);
		delete tvi;
		tvi = dynamic_cast<KTorrentViewItem*>(currentItem());
		if (tvi)
			currentChanged(tvi->getTC());
		else
			currentChanged(0);
	}
}


void KTorrentView::update()
{
	QMap<bt::TorrentControl*,KTorrentViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		KTorrentViewItem* tvi = i.data();
		tvi->update();
		i++;
	}
}

void KTorrentView::onTrackerError(bt::TorrentControl* tc,const QString & err)
{
	KMessageBox::error(this,err,i18n("Error"));
	tc->stop();
}

#include "ktorrentview.moc"
