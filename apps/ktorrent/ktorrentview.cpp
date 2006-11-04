/***************************************************************************
 *   Copyright (C) 2005,2006 by                                            *
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
#include <kurl.h>
#include <kurldrag.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>

#include <interfaces/torrentinterface.h>
#include <torrent/globals.h>
#include <util/log.h>

#include <groups/group.h>
#include <groups/torrentdrag.h>

#include "ktorrentview.h"
#include "ktorrentviewitem.h"
#include "settings.h"
#include "scandialog.h"
#include "addpeerwidget.h"

using namespace bt;
using namespace kt;

KTorrentView::KTorrentView(QWidget *parent)
	: KListView(parent),menu(0),current_group(0)
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

	restoreLayout(KGlobal::config(),"KTorrentView");
	setDragEnabled(true);
}

KTorrentView::~KTorrentView()
{
}

void KTorrentView::setCurrentGroup(Group* group)
{
	if (current_group == group)
		return;
	
	current_group = group;
	
	if (current_group)
		setCaption(current_group->groupName());
	else
		setCaption(i18n("All Torrents"));
	
	// go over the current items, if they still match keep them, else remove them
	// add new itesm if necessary
	QMap<TorrentInterface*,KTorrentViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		KTorrentViewItem* tvi = i.data();
		TorrentInterface* tc = i.key();
		if (current_group && !current_group->isMember(tc))
		{
			if (tvi)
			{
				delete tvi;
				i.data() = 0;
			}
		}
		else if (!tvi)
		{
			tvi = new KTorrentViewItem(this,tc);
			i.data() = tvi;
		}

		i++;
	}
	
	onExecuted(currentItem());
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
			iload->loadIconSet("ktremove",KIcon::Small),i18n("Remove Torrent"),
			this,SLOT(removeDownloads()));
	
	remove_all_id = menu->insertItem(
			iload->loadIconSet("ktremove",KIcon::Small),i18n("Remove Torrent and Data"),
			this,SLOT(removeDownloadsAndData()));
	
	queue_id = menu->insertItem(
			iload->loadIconSet("player_playlist",KIcon::Small),i18n("Enqueue/Dequeue"),
			this,SLOT(queueSlot()));
	
	menu->insertSeparator();
	
	add_peer_id = menu->insertItem(
			iload->loadIconSet("add", KIcon::Small), i18n("Add Peers..."),
			this, SLOT(showAddPeersWidget())); 
	
	peer_sources_menu = new KPopupMenu(menu);
	peer_sources_id = menu->insertItem(i18n("Additional Peer Sources"), peer_sources_menu);
	peer_sources_menu->insertTitle(i18n("Torrent peer sources:"));
	peer_sources_menu->setCheckable(true);
	dht_id = peer_sources_menu->insertItem(i18n("DHT"), this, SLOT(dhtSlot()));
	
	menu->insertSeparator();
	
	announce_id = menu->insertItem(
			iload->loadIconSet("apply",KIcon::Small),i18n("Manual Announce"),
			this,SLOT(manualAnnounce())); 
	
	preview_id = menu->insertItem(
			iload->loadIconSet("frame_image",KIcon::Small),i18n("Preview"), 
			this, SLOT(previewFiles()));
	
	menu->insertSeparator();
	dirs_sub_menu = new KPopupMenu(menu);
	dirs_id = menu->insertItem(i18n("Open Directory..."), dirs_sub_menu);
	outputdir_id = dirs_sub_menu->insertItem(iload->loadIconSet("folder",KIcon::Small), i18n("Data Directory"), this, SLOT(openOutputDirectory()));
	torxdir_id = dirs_sub_menu->insertItem(iload->loadIconSet("folder",KIcon::Small), i18n("Temporary Directory"), this, SLOT(openTorXDirectory()));
	
	menu->insertSeparator();
	remove_from_group_id =  menu->insertItem(i18n("Remove From Group"),this, SLOT(removeFromGroup()));
	groups_sub_menu = new KPopupMenu(menu);
	
	add_to_group_id = menu->insertItem(i18n("Add to Group"),groups_sub_menu);
	
	menu->insertSeparator();
	scan_id = menu->insertItem(i18n("Check Data Integrity"),this, SLOT(checkDataIntegrity()));
	
}

void KTorrentView::saveSettings()
{
	saveLayout(KGlobal::config(),"KTorrentView");
	KGlobal::config()->sync();
}


int KTorrentView::getNumRunning()
{
	int num = 0;
	QMap<TorrentInterface*,KTorrentViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		KTorrentViewItem* tvi = i.data();
		if (tvi)
		{
			TorrentInterface* tc = tvi->getTC();
			num += tc->getStats().running ? 1 : 0;
		}
		i++;
	}
	return num;
}

bool KTorrentView::startDownload(kt::TorrentInterface* tc)
{
	if (tc && !tc->getStats().running)
	{
		wantToStart(tc);
		if (!tc->getStats().running && !tc->getStats().stopped_by_error)
		{
			if (tc->getStats().completed)
			{
				if (!tc->overMaxRatio())
					return false;
			}
			else
				return false;
		}
	}
	return true;
}

void KTorrentView::stopDownload(kt::TorrentInterface* tc)
{
	if (tc && tc->getStats().running)
		wantToStop(tc,true);
}

void KTorrentView::showStartError()
{
	QString err = i18n("Cannot start more than 1 download, ",
					   "Cannot start more than %n downloads, ",Settings::maxDownloads());
		
	err += i18n("and 1 seed. ","and %n seeds. ",Settings::maxSeeds());
	err += i18n("Go to Settings -> Configure KTorrent, if you want to change the limits.");
	KMessageBox::error(this,err);
}

void KTorrentView::startDownloads()
{
	bool err = false;
	
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (!startDownload(tc))
			err = true;;
	}

	if (err)
	{
		showStartError();
	}
	
	// make sure toolbuttons get updated
	onSelectionChanged();
}
	
void KTorrentView::stopDownloads()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		stopDownload(kvi->getTC());
	}
	
	// make sure toolbuttons get updated
	onSelectionChanged();
}

void KTorrentView::startAllDownloads()
{
	bool err = false;
	QMap<TorrentInterface*,KTorrentViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		KTorrentViewItem* tvi = i.data();
		if (tvi && !startDownload(tvi->getTC()))
			err = true;
			
		i++;
	}
	
	if (err)
	{
		showStartError();
	}
}

void KTorrentView::stopAllDownloads()
{
	QMap<TorrentInterface*,KTorrentViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		KTorrentViewItem* tvi = i.data();
		if (tvi)
			stopDownload(tvi->getTC());
	
		i++;
	}
}
	
void KTorrentView::removeDownloads()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		bool dummy = false;
		if (tc && !tc->isCheckingData(dummy))
		{	
			const TorrentStats & s = tc->getStats();
			bool data_to = false;
			if (s.bytes_left_to_download > 0)
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
	
	// make sure toolbuttons get updated
	onSelectionChanged();
}

void KTorrentView::removeDownloadsAndData()
{
	QString msg = i18n("You will lose all the downloaded data. Are you sure you want to do this?");
        // TODO: replace i18n("Remove") by KStdGuiItem::remove() in KDE4
	if (KMessageBox::warningYesNo(this,msg, i18n("&Remove"), KStdGuiItem::cancel()) == KMessageBox::No)
		return;
	
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc)
			wantToRemove(tc,true);
	}
	
	// make sure toolbuttons get updated
	onSelectionChanged();
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
		TorrentInterface* cur = i.key();
		if (tc == cur)
			break;
		i++;
	}
	
	if (tvi)
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
	else
	{
		currentChanged(0);
	}
}

void KTorrentView::showContextMenu(KListView* ,QListViewItem*,const QPoint & p)
{
	bool en_start = false;
	bool en_stop = false;
	bool en_remove = false;
	bool en_prev = false;
	bool en_announce = false;
	bool en_add_peer = false;
	bool en_dirs = false;
	bool en_peer_sources = false;
	bool dummy = false;
	
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc)
		{
			const TorrentStats & s = tc->getStats();
			
			if (tc->readyForPreview() && !s.multi_file_torrent)
				en_prev = true;
			
			if (!tc->isCheckingData(dummy))
				en_remove = true;
			
			if (!s.running)
			{
				if (!tc->isCheckingData(dummy))
				{
					en_start = true;
				}
			}
			else
			{
				if (!tc->isCheckingData(dummy))
				{
					en_stop = true;
					if (tc->announceAllowed())
						en_announce = true;
				}
			}
			
			if (!s.priv_torrent && !tc->isCheckingData(dummy))
			{
				en_add_peer = true;
				en_peer_sources = true;
			}
		}
	}
	
	en_add_peer = en_add_peer && en_stop;
	
	menu->setItemEnabled(start_id,en_start);
	menu->setItemEnabled(stop_id,en_stop);
	menu->setItemEnabled(remove_id,en_remove);
	menu->setItemEnabled(remove_all_id,en_remove);
	menu->setItemEnabled(preview_id,en_prev);
	menu->setItemEnabled(add_peer_id, en_add_peer);
	menu->setItemEnabled(announce_id,en_announce);
	menu->setItemEnabled(queue_id, en_remove);
	
	menu->setItemEnabled(remove_from_group_id,current_group && !current_group->isStandardGroup());
	menu->setItemEnabled(add_to_group_id,groups_sub_menu->count() > 0);
	
	if (sel.count() == 1)
	{
		//enable directories
		en_dirs = true;
		
		KTorrentViewItem* kvi = (KTorrentViewItem*)sel.getFirst();
		TorrentInterface* tc = kvi->getTC();
		// no data check when we are preallocating diskspace
		if (tc->getStats().status == kt::ALLOCATING_DISKSPACE || tc->isCheckingData(dummy))
			menu->setItemEnabled(scan_id, false);
		else
			menu->setItemEnabled(scan_id, true);
		
		//enable additional peer sources if torrent is not private
		menu->setItemEnabled(peer_sources_id, en_peer_sources);
		
		if (en_peer_sources)
			peer_sources_menu->setItemChecked(dht_id, tc->dhtStarted());
	}
	else
	{
		menu->setItemEnabled(scan_id,false);
		
		//disable peer source
		menu->setItemEnabled(peer_sources_id, false);	
	}
	
	menu->setItemEnabled(dirs_id, en_dirs);
	
	menu->popup(p);
}

void KTorrentView::addTorrent(TorrentInterface* tc)
{
	if (current_group && !current_group->isMember(tc))
	{
		items.insert(tc,0);
	}
	else
	{
		KTorrentViewItem* tvi = new KTorrentViewItem(this,tc);
		items.insert(tc,tvi);
		tvi->update();
		if (items.count() == 1)
			currentChanged(tc);
	}
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
	QMap<kt::TorrentInterface*,KTorrentViewItem*>::iterator i = items.begin();
	while (i != items.end())
	{
		KTorrentViewItem* tvi = i.data();
		if (tvi)
			tvi->update();
		
		
		// check if the torrent still is part of the group
		kt::TorrentInterface* ti = i.key();
		if (tvi && current_group && !current_group->isMember(ti))
		{
			// torrent is no longer a member of this group so remove it from the view
			delete tvi;
			i.data() = 0;
		}
		else if (!tvi && (!current_group || current_group->isMember(ti)))
		{
			tvi = new KTorrentViewItem(this,ti);
			i.data() = tvi;
		}
		
		i++;
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
		bool dummy;
		if (tc && !tc->isCheckingData(dummy))
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
		bool dummy;
		if (tc && !tc->isCheckingData(dummy))
			emit queue(tc);
	}
}


void KTorrentView::checkDataIntegrity()
{	
	QPtrList<QListViewItem> sel = selectedItems();
	if (sel.count() == 0)
		return;
	
	bool dummy = false;
	KTorrentViewItem* kvi = (KTorrentViewItem*)sel.first();
	TorrentInterface* tc = kvi->getTC();
	if (!tc->isCheckingData(dummy))
	{
		needsDataCheck(tc);
	}
	else
	{
		KMessageBox::error(0,i18n("You are already checking the data of the torrent %1 !").arg(tc->getStats().torrent_name));
	}
}

QDragObject* KTorrentView::dragObject()
{
	QPtrList<QListViewItem> sel = selectedItems();
	if (sel.count() == 0)
		return 0;
	 
	return new TorrentDrag(this);
}

void KTorrentView::getSelection(QPtrList<kt::TorrentInterface> & sel)
{
	QPtrList<QListViewItem> s = selectedItems();
	if (s.count() == 0)
		return;
	
	QPtrList<QListViewItem>::iterator i = s.begin();
	while (i != s.end())
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*i;
		TorrentInterface* tc = kvi->getTC();
		sel.append(tc);
		i++;
	}
}

void KTorrentView::removeFromGroup()
{
	QPtrList<QListViewItem> s = selectedItems();
	if (s.count() == 0 || !current_group || current_group->isStandardGroup())
		return;
	
	QPtrList<QListViewItem>::iterator i = s.begin();
	while (i != s.end())
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*i;
		TorrentInterface* tc = kvi->getTC();
		current_group->removeTorrent(tc);
		delete kvi;
		items[tc] = 0;
		i++;
	}
}

void KTorrentView::addSelectionToGroup(kt::Group* g)
{
	QPtrList<QListViewItem> s = selectedItems();
	if (s.count() == 0 || !g)
		return;
	
	QPtrList<QListViewItem>::iterator i = s.begin();
	while (i != s.end())
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*i;
		TorrentInterface* tc = kvi->getTC();
		g->addTorrent(tc);
		i++;
	}
}

void KTorrentView::showAddPeersWidget()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		bool dummy;
		if (tc && !tc->isCheckingData(dummy))
		{
			AddPeerWidget dlg(tc, this);
			dlg.exec();
		}
	}
}

void KTorrentView::openOutputDirectory()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc)
		{
			if(tc->getStats().multi_file_torrent)
				new KRun(KURL::fromPathOrURL(tc->getDataDir() + tc->getStats().torrent_name), 0, true, true);
			else
				new KRun(KURL::fromPathOrURL(tc->getDataDir()), 0, true, true);
		}
	}
}

void KTorrentView::openTorXDirectory()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc)
		{
			new KRun(KURL::fromPathOrURL(tc->getTorDir()), 0, true, true);
		}
	}
}

void KTorrentView::dhtSlot()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		bool dummy;
		if (tc && !tc->isCheckingData(dummy))
		{
			if(tc->dhtStarted())
				tc->stopDHT();
			else
				tc->startDHT();
		}
	}
}

#include "ktorrentview.moc"
