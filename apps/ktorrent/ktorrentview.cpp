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
#include <kfiledialog.h>

#include <interfaces/torrentinterface.h>
#include <torrent/globals.h>
#include <util/log.h>

#include <groups/group.h>
#include <groups/torrentdrag.h>

#include <qcursor.h>
#include <qheader.h>
#include <qvaluelist.h>
		
#include "ktorrentview.h"
#include "ktorrentviewitem.h"
#include "settings.h"
#include "scandialog.h"
#include "addpeerwidget.h"
#include "ktorrentviewmenu.h"

using namespace bt;
using namespace kt;

KTorrentView::KTorrentView(QWidget *parent)
	: KListView(parent),menu(0),current_group(0)
{

	setupColumns();
	
	connect(this,SIGNAL(executed(QListViewItem* )),
			this,SLOT(onExecuted(QListViewItem* )));
	
	connect(this,SIGNAL(currentChanged(QListViewItem* )),
			this,SLOT(onExecuted(QListViewItem* )));
	
	connect(this,SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint& )),
			this,SLOT(showContextMenu(KListView*, QListViewItem*, const QPoint& )));
	
	connect(this,SIGNAL(selectionChanged()),this,SLOT(onSelectionChanged()));

	menu = new KTorrentViewMenu(this);
	connect(menu,SIGNAL(groupItemActivated(const QString&)),this,SLOT(gsmItemActived(const QString&)));
	
	connect(m_headerMenu, SIGNAL(activated(int)), this, SLOT(onColumnVisibilityChange( int )));

	setFrameShape(QFrame::NoFrame);
}

KTorrentView::~KTorrentView()
{
}

void KTorrentView::insertColumn(QString label, Qt::AlignmentFlags align)
{
	m_headerMenu->insertItem(label);
		
	int ind = addColumn(label);
	setColumnAlignment(ind, align);
}

void KTorrentView::setupColumns()
{
		//Header menu
	m_headerMenu = new KPopupMenu(this);
	m_headerMenu->setCheckable(true);
	m_headerMenu->insertTitle(i18n("Visible columns"));
	
	insertColumn(i18n("File"), Qt::AlignLeft);
	insertColumn(i18n("Status"), Qt::AlignLeft);
	insertColumn(i18n("Downloaded"), Qt::AlignRight);
	insertColumn(i18n("Size"), Qt::AlignRight); 
	insertColumn(i18n("Uploaded"), Qt::AlignRight);
	insertColumn(i18n("Down Speed"), Qt::AlignRight);
	insertColumn(i18n("Up Speed"), Qt::AlignRight);
	insertColumn(i18n("Time Left"), Qt::AlignCenter);
	insertColumn(i18n("Peers"), Qt::AlignRight);
	insertColumn(i18n("% Complete"), Qt::AlignRight);
	insertColumn(i18n("Share ratio"), Qt::AlignRight);
	
	setAllColumnsShowFocus(true);
	setShowSortIndicator(true);
	setAcceptDrops(true);
	setSelectionMode(QListView::Extended);
	for (Uint32 i = 0;i < (Uint32)columns();i++)
	{
		setColumnWidth(i, 100);
		setColumnWidthMode(i,QListView::Manual);
	}
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

void KTorrentView::saveSettings(KConfig* cfg,int idx)
{
	saveLayout(cfg,QString("KTorrentView-%1").arg(idx));
}


void KTorrentView::loadSettings(KConfig* cfg,int idx)
{
	restoreLayout(cfg,QString("KTorrentView-%1").arg(idx));
	setDragEnabled(true);

	for(int i=0; i<columns();++i)
	{
		bool visible = columnVisible(i);
		
		m_headerMenu->setItemChecked(m_headerMenu->idAt(i+1), visible);
		header()->setResizeEnabled(visible, i);
	}
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
				int ret = KMessageBox::questionYesNoCancel(
						this,msg,i18n("Remove Download"),
						i18n("Delete Data"),i18n("Keep Data"));
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
	if (KMessageBox::warningYesNo(this,msg, i18n("Remove Torrent"), i18n("&Remove"), 
            KStdGuiItem::cancel()) == KMessageBox::No)
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
	updateGroupsSubMenu(menu->getGroupsSubMenu());
	menu->show(p);
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

void KTorrentView::getSelection(QValueList<kt::TorrentInterface*> & sel)
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

void KTorrentView::setDownloadLocationSlot()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		if (tc)
		{
			QString dn;
			TorrentStats s =  tc->getStats();
			
			if(s.multi_file_torrent)
			{
				dn = KFileDialog::getExistingDirectory(s.output_path, this, i18n("Choose download location for %1").arg(tc->getStats().torrent_name));
								
				if(dn.isNull() || dn.isEmpty())
					continue;
								
				if(!dn.endsWith("/"))
					dn += "/";
				
				dn += s.torrent_name;
			}
			else
			{
				dn =  KFileDialog::getSaveFileName(s.output_path, QString::null, this, i18n("Choose download location for %1").arg(tc->getStats().torrent_name));
			}
						
			if(!dn.isEmpty())
				tc->changeOutputDir(dn);
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
			bool on = tc->isFeatureEnabled(kt::DHT_FEATURE);
			tc->setFeatureEnabled(kt::DHT_FEATURE,!on);
		}
	}
}

void KTorrentView::utPexSlot()
{
	QPtrList<QListViewItem> sel = selectedItems();
	for (QPtrList<QListViewItem>::iterator itr = sel.begin(); itr != sel.end();itr++)
	{
		KTorrentViewItem* kvi = (KTorrentViewItem*)*itr;
		TorrentInterface* tc = kvi->getTC();
		bool dummy;
		if (tc && !tc->isCheckingData(dummy))
		{
			bool on = tc->isFeatureEnabled(kt::UT_PEX_FEATURE);
			tc->setFeatureEnabled(kt::UT_PEX_FEATURE,!on);
		}
	}
}


void KTorrentView::columnHide(int index)
{
	hideColumn(index);
	header()->setResizeEnabled(FALSE, index);
}

void KTorrentView::columnShow(int index)
{
	setColumnWidth(index, 100);
	header()->setResizeEnabled(TRUE, index);
}

bool KTorrentView::columnVisible(int index)
{
	return columnWidth(index) != 0;
}

void KTorrentView::onColumnVisibilityChange(int id)
{
	int mid = m_headerMenu->indexOf(id) - 1;
	if(mid == -1)
		return;

	bool visible = !columnVisible(mid);
	
	m_headerMenu->setItemChecked(id, visible);
	
	if(visible)
		columnShow(mid);
	else
		columnHide(mid);
}

bool KTorrentView::eventFilter(QObject* watched, QEvent* e)
{
	if((QHeader*)watched == header())
	{
		switch(e->type())
		{
				case QEvent::MouseButtonPress:
				{
					if(static_cast<QMouseEvent *>(e)->
					        button() == RightButton)
						m_headerMenu->popup(QCursor::pos());

					break;
				}
				default:
				break;
		}
	}

	return KListView::eventFilter(watched, e);
}

void KTorrentView::gsmItemActived(const QString & group)
{
	groupsSubMenuItemActivated(this,group);
}

#include "ktorrentview.moc"
