/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kurldrag.h>
#include <kmessagebox.h>
#include <interfaces/torrentinterface.h>
#include <torrent/globals.h>
#include <util/log.h>
#include <kmessagebox.h>
#include "ktorrentview.h"
#include "ktorrentviewitem.h"
#include "settings.h"
#include "scandialog.h"



using namespace bt;
using namespace kt;

KTorrentView::KTorrentView(QWidget *parent, bool seed_view)
	: KListView(parent),m_seedView(seed_view),
	show_debug_view(false),menu(0)
{
	addColumn(i18n("File"));
	addColumn(i18n("Status"));
	addColumn(i18n("Downloaded"));
	addColumn(i18n("Size")); 
	addColumn(i18n("Uploaded"));
	addColumn(i18n("Down Speed"));
	addColumn(i18n("Up Speed"));
	addColumn(i18n("Time Left"));
	addColumn(i18n("Peers"));
	addColumn(i18n("% Complete"));

	connect(this,SIGNAL(currentChanged(QListViewItem* )),
			this,SLOT(onExecuted(QListViewItem* )));
	show_debug_view = bt::Globals::instance().isDebugModeSet();
	connect(this,SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint& )),
			this,SLOT(showContextMenu(KListView*, QListViewItem*, const QPoint& )));
	
	connect(this,SIGNAL(selectionChanged()),this,SLOT(onSelectionChanged()));

	makeMenu();

	setAllColumnsShowFocus(true);

	setColumnWidth(0,200);
	setColumnWidthMode(0,QListView::Manual);
	setColumnWidth(1,100);
	setColumnWidthMode(1,QListView::Manual);
	setShowSortIndicator(true);
	setAcceptDrops(true);
	setSelectionMode(QListView::Extended);
	for (Uint32 i = 2;i < (Uint32)columns();i++)
		setColumnWidthMode(i,QListView::Manual);

	restoreLayout(KGlobal::config(),m_seedView ? "KTorrentSeedView" : "KTorrentView");
}

KTorrentView::~KTorrentView()
{
}

void KTorrentView::makeMenu()
{
	KIconLoader* iload = KGlobal::iconLoader();
	menu = new KPopupMenu(this);
	
	stop_id = menu->insertItem(
			iload->loadIconSet("ktstop",KIcon::Small),i18n("to stop", "Stop"),
			this,SLOT(stopDownloads()));

	start_id = menu->insertItem(
			iload->loadIconSet("ktstart",KIcon::Small),i18n("to start", "Start"),
			this,SLOT(startDownloads()));

	remove_id = menu->insertItem(
			iload->loadIconSet("ktremove",KIcon::Small),i18n("Remove"),
			this,SLOT(removeDownloads()));
	
	queue_id = menu->insertItem(
			iload->loadIconSet("player_playlist",KIcon::Small),i18n("Enqueue/Dequeue"),
			this,SLOT(queueSlot()));
	
	menu->insertSeparator();

	announce_id = menu->insertItem(
			iload->loadIconSet("apply",KIcon::Small),i18n("Manual Announce"),
			this,SLOT(manualAnnounce())); 
	
	preview_id = menu->insertItem(
			iload->loadIconSet("frame_image",KIcon::Small),i18n("Preview"), 
			this, SLOT(previewFiles())); 
	menu->insertSeparator();
	scan_id = menu->insertItem(i18n("Check Data Integrity"),this, SLOT(checkDataIntegrity()));
}

void KTorrentView::saveSettings()
{
	saveLayout(KGlobal::config(),m_seedView ? "KTorrentSeedView" : "KTorrentView");
	KGlobal::config()->sync();
}

void KTorrentView::setShowDebugView(bool yes)
{
	show_debug_view = yes;
}

int KTorrentView::getNumRunning()
{
	int num = 0;
	QMap<TorrentInterface*,KTorrentViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		KTorrentViewItem* tvi = i.data();
		TorrentInterface* tc = tvi->getTC();
		num += tc->getStats().running ? 1 : 0;
		i++;
	}
	return num;
}

void KTorrentView::startDownloads()
{
	bool err_seed = false, err_down = false;
	
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc && !tc->getStats().running)
		{
			wantToStart(tc);
			if (!tc->getStats().running && !tc->getStats().stopped_by_error)
			{
				if (tc->getStats().completed)
				{
					if (!tc->overMaxRatio())
						err_seed = true;
				}
				else
					err_down = true;
			}
		}
	}

	// downloads and seeds are in two separate views so 
	// either err_seed is true or err_down is true or none are true
	if (err_down)
		KMessageBox::error(this,
						   i18n("Cannot start more than 1 download."
								   " Go to Settings -> Configure KTorrent,"
								   " if you want to change the limit.",
						   "Cannot start more than %n downloads."
								   " Go to Settings -> Configure KTorrent,"
								   " if you want to change the limit.",
						   Settings::maxDownloads()),
						   i18n("Error"));
	else if (err_seed)
		KMessageBox::error(this,
						   i18n("Cannot start more than 1 seed."
								   " Go to Settings -> Configure KTorrent,"
								   " if you want to change the limit.",
						   "Cannot start more than %n seeds."
								   " Go to Settings -> Configure KTorrent,"
								   " if you want to change the limit.",
						   Settings::maxSeeds()),
						   i18n("Error"));

}
	
void KTorrentView::stopDownloads()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc && tc->getStats().running)
			wantToStop(tc,true);
	}
}
	
void KTorrentView::removeDownloads()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc)
		{	
			const TorrentStats & s = tc->getStats();
			bool data_to = false;
			if (s.bytes_left > 0)
			{
				QString msg = i18n("The torrent %1 has not finished downloading, "
						"do you want to delete the incomplete data, too?").arg(s.torrent_name);
				int ret = KMessageBox::questionYesNoCancel(this,msg,i18n("Remove Download"));
				if (ret == KMessageBox::Cancel)
					return;
				else if (ret == KMessageBox::Yes)
					data_to = true;
			}
			wantToRemove(tc,data_to);
		}
	}
}

void KTorrentView::manualAnnounce()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc && tc->getStats().running)
			tc->updateTracker();
	}
}

void KTorrentView::previewFiles() 
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc && tc->readyForPreview() && !tc->getStats().multi_file_torrent)
		{
			new KRun(KURL::fromPathOrURL(tc->getTorDir()+"cache"), true, true);
		}
	}
}

TorrentInterface* KTorrentView::getCurrentTC()
{
	KTorrentViewItem* tvi = dynamic_cast<KTorrentViewItem*>(currentItem());
	if (tvi)
		return tvi->getTC();
	else
		return 0;
}

QCStringList KTorrentView::getTorrentInfo(kt::TorrentInterface* tc)
{
	QCStringList torrentinfo;
        KTorrentViewItem* tvi = 0;
        QMap<TorrentInterface*,KTorrentViewItem*>::iterator i = items.begin();
        while (i != items.end())
        {
                tvi = i.data();
		TorrentInterface* cur = tvi->getTC();
		if(tc == cur)
			break;
                i++;
        }
	if(tvi)
		for(int i = 0; i < 10; i++)
			torrentinfo.append(tvi->text(i).ascii());
	return torrentinfo;
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

void KTorrentView::showContextMenu(KListView* ,QListViewItem*,const QPoint & p)
{
	bool en_start = false;
	bool en_stop = false;
	bool en_remove = false;
	bool en_prev = false;
	bool en_announce = true;
	
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc)
		{
			const TorrentStats & s = tc->getStats();
			if (!s.running)
			{
				en_start = true;
				en_announce = false;
			}
			else
			{
				en_stop = true;
				if(!tc->announceAllowed())
					en_announce = false;
			}
			
			if (tc->readyForPreview() && !s.multi_file_torrent)
				en_prev = true;
		}
	}
	
	en_remove = sel.count() > 0;
	menu->setItemEnabled(start_id,en_start);
	menu->setItemEnabled(stop_id,en_stop);
	menu->setItemEnabled(remove_id,en_remove);
	menu->setItemEnabled(preview_id,en_prev);
	menu->setItemEnabled(announce_id,en_announce);
	menu->setItemEnabled(queue_id, en_remove);
	
	if (sel.count() == 1)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)sel.getFirst();
		TorrentInterface* tc = kvi->getTC();
		// no data check when we are preallocating diskspace
		menu->setItemEnabled(scan_id, 
							 tc->getStats().status != kt::ALLOCATING_DISKSPACE);
	}
	else
	{
		menu->setItemEnabled(scan_id,false);
	}
	menu->popup(p);
}

void KTorrentView::addTorrent(TorrentInterface* tc)
{
	if(m_seedView && !tc->getStats().completed)
		return;
	if(!m_seedView && tc->getStats().completed)
		return;
	
	KTorrentViewItem* tvi = new KTorrentViewItem(this,tc);
	items.insert(tc,tvi);
	tvi->update();
	if (items.count() == 1)
		currentChanged(tc);
}

void KTorrentView::removeTorrent(TorrentInterface* tc)
{
	QMap<kt::TorrentInterface*,KTorrentViewItem*>::iterator i = items.find(tc);
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
	kt::TorrentInterface* tc = 0l;
	QMap<kt::TorrentInterface*,KTorrentViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		KTorrentViewItem* tvi = i.data();
		tvi->update();
		//check if seeded torrent is activated and move it to downloadView
		kt::TorrentInterface* ti = i.key();
		if(!ti->getStats().completed && m_seedView)
			tc = ti;
		i++;
	}
	if(tc)
	{
		QMap<kt::TorrentInterface*,KTorrentViewItem*>::iterator i = items.find(tc);
		if (i != items.end())
		{
			items.remove(i);
			delete i.data();
		}
		emit viewChange(tc);
		if(items.count() == 0)
			emit currentChanged(0l);
		Out(SYS_GEN|LOG_NOTICE) << "Torrent moved to DownloadView." << endl;
	}
	sort();
}

bool KTorrentView::acceptDrag(QDropEvent* event) const
{
	// accept uri drops only
	return KURLDrag::canDecode(event);
}

void KTorrentView::torrentFinished(kt::TorrentInterface* tc)
{
	if(m_seedView)
		return;
	
	QMap<kt::TorrentInterface*,KTorrentViewItem*>::iterator i = items.find(tc);
	if (i != items.end())
	{
		items.remove(i);
		delete i.data();
	}
	emit viewChange(tc);
	if(items.count() == 0)
		emit currentChanged(0l);
	Out(SYS_GEN|LOG_NOTICE) << "Torrent moved to SeedView." << endl;
}

void KTorrentView::onSelectionChanged()
{
	bool en_start = false;
	bool en_stop = false;
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc)
		{
			const TorrentStats & s = tc->getStats();
			if (!s.running)
				en_start = true;
			else
				en_stop = true;
		}
	}
	
	updateActions(en_start,en_stop,sel.count() > 0,sel.count() == 1);
}

void KTorrentView::queueSlot()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc)
			emit queue(tc);
	}
}


void KTorrentView::checkDataIntegrity()
{	
	QPtrList<QListViewItem> sel = selectedItems();
	if (sel.count() == 0)
		return;
	
	KTorrentViewItem* kvi = (KTorrentViewItem*)sel.first();
	TorrentInterface* tc = kvi->getTC();
	ScanDialog* scan_dlg = new ScanDialog(false,this);
	scan_dlg->setCaption(i18n("Checking Data Integrity"));
	scan_dlg->show();
	scan_dlg->execute(tc,false);
	scan_dlg->deleteLater();
}


#include "ktorrentview.moc"
