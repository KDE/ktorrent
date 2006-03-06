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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
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



using namespace bt;
using namespace kt;

KTorrentView::KTorrentView(QWidget *parent, bool seed_view)
	: KListView(parent),m_seedView(seed_view),show_debug_view(false),menu(0),curr(0)

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

	KIconLoader* iload = KGlobal::iconLoader();
	menu = new KPopupMenu(this);
	
	stop_id = menu->insertItem(
			iload->loadIconSet("ktstop",KIcon::Small),i18n("to stop", "Stop"),
			this,SLOT(stopDownload()));

	start_id = menu->insertItem(
			iload->loadIconSet("ktstart",KIcon::Small),i18n("to start", "Start"),
			this,SLOT(startDownload()));

	remove_id = menu->insertItem(
			iload->loadIconSet("ktremove",KIcon::Small),i18n("Remove"),
			this,SLOT(removeDownload()));
	menu->insertSeparator();

    menu->insertItem(iload->loadIconSet("apply",KIcon::Small),i18n("Manual Announce"),this,SLOT(manualAnnounce())); 
    preview_id = menu->insertItem(iload->loadIconSet("frame_image",KIcon::Small),i18n("Preview"), this, SLOT(previewFile())); 

	setAllColumnsShowFocus(true);

	setColumnWidth(0,200);
	setColumnWidthMode(0,QListView::Manual);
	setColumnWidth(1,100);
	setColumnWidthMode(1,QListView::Manual);
	setShowSortIndicator(true);
	setAcceptDrops(true);
	
	for (Uint32 i = 2;i < (Uint32)columns();i++)
		setColumnWidthMode(i,QListView::Manual);

	restoreLayout(KGlobal::config(),m_seedView ? "KTorrentSeedView" : "KTorrentView");
}

KTorrentView::~KTorrentView()
{
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

void KTorrentView::startDownload()
{
	if (!curr)
		return;

	TorrentInterface* tc = curr->getTC();
	if (tc && !tc->getStats().running)
	{
		wantToStart(tc);
		if (!tc->getStats().running && !tc->getStats().stopped_by_error)
		{
			bool seed = tc->getStats().completed;
			int nr = seed ? Settings::maxSeeds() : Settings::maxDownloads();
			
			if(!seed)
				KMessageBox::error(this,
								   i18n("Cannot start more than 1 download."
										   " Go to Settings -> Configure KTorrent,"
										   " if you want to change the limit.",
								   "Cannot start more than %n downloads."
										   " Go to Settings -> Configure KTorrent,"
										   " if you want to change the limit.",
								   nr),
								   i18n("Error"));
			else
				KMessageBox::error(this,
								   i18n("Cannot start more than 1 seed."
										   " Go to Settings -> Configure KTorrent,"
										   " if you want to change the limit.",
								   "Cannot start more than %n seeds."
										   " Go to Settings -> Configure KTorrent,"
										   " if you want to change the limit.",
								   nr),
								   i18n("Error"));
		}
	}
}
	
void KTorrentView::stopDownload()
{
	if (!curr)
		return;

	TorrentInterface* tc = curr->getTC();
	wantToStop(tc,true);
}
	
void KTorrentView::removeDownload()
{
	if (!curr)
		return;

	TorrentInterface* tc = curr->getTC();
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

void KTorrentView::manualAnnounce()
{
	if (!curr)
		return;

	curr->getTC()->updateTracker();
}

void KTorrentView::previewFile() 
{
    if (!curr) 
        return; 

	new KRun(curr->getTC()->getTorDir()+"cache", true, true);
}

TorrentInterface* KTorrentView::getCurrentTC()
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
		TorrentInterface* tc = curr->getTC();
		const TorrentStats & s = tc->getStats();
		menu->setItemEnabled(start_id,!s.running);
		menu->setItemEnabled(stop_id,s.running);
		menu->setItemEnabled(remove_id,true);
		menu->setItemEnabled(preview_id, tc->readyForPreview() && !s.multi_file_torrent);
		menu->popup(p);
	}
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
		emit currentChanged(0l);
		emit viewChange(tc);
		Out() << "Torrent moved to DownloadView." << endl;
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
	emit currentChanged(0l);
	emit viewChange(tc);
	Out() << "Torrent moved to SeedView." << endl;
}


#include "ktorrentview.moc"
