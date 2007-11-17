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
#include <QFileInfo>
#include <krun.h>
#include <klocale.h>
#include <ksharedconfig.h>
#include <kmessagebox.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <util/functions.h>
#include <interfaces/functions.h>
#include <groups/group.h>
#include "view.h"
#include "core.h"
#include "viewitem.h"
#include "viewmenu.h"
#include "scandlg.h"
#include "speedlimitsdlg.h"
#include "addpeersdlg.h"

using namespace bt;

namespace kt
{
	
	View::View(Core* core,QWidget* parent) : QTreeWidget(parent),core(core),group(0),num_torrents(0),num_running(0)
	{
		menu = new ViewMenu(core->getGroupManager(),this);
		setContextMenuPolicy(Qt::CustomContextMenu);
		setRootIsDecorated(false);
		setSortingEnabled(true);
		setAlternatingRowColors(true);
		setSelectionMode(QAbstractItemView::ExtendedSelection);
		setSelectionBehavior(QAbstractItemView::SelectRows);

		QStringList columns;
		columns << i18n("Name") 
			<< i18n("Status") 
			<< i18n("Downloaded")
			<< i18n("Size")
			<< i18n("Uploaded") 
			<< i18n("Down Speed")
			<< i18n("Up Speed")
			<< i18n("Time Left")
			<< i18n("Seeders")
			<< i18n("Leechers")
			// xgettext: no-c-format
			<< i18n("% Complete") 
			<< i18n("Share Ratio")
			<< i18n("Time Downloaded")
			<< i18n("Time Seeded");
		setHeaderLabels(columns);
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),this,SLOT(addTorrent(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),this,SLOT(removeTorrent(bt::TorrentInterface*)));
		connect(this,SIGNAL(wantToRemove(bt::TorrentInterface*,bool )),core,SLOT(remove(bt::TorrentInterface*,bool )));
		connect(this,SIGNAL(wantToStart( bt::TorrentInterface* )),core,SLOT(start( bt::TorrentInterface* )));
		connect(this,SIGNAL(wantToStop( bt::TorrentInterface*, bool )),core,SLOT(stop( bt::TorrentInterface*, bool )));
		connect(this,SIGNAL(customContextMenuRequested(const QPoint & ) ),this,SLOT(showMenu( const QPoint& )));
		connect(this,SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
			this,SLOT(onCurrentItemChanged(QTreeWidgetItem * ,QTreeWidgetItem *)));


		kt::QueueManager* qman = core->getQueueManager();
		for (QList<bt::TorrentInterface*>::iterator i = qman->begin();i != qman->end();i++)
			addTorrent(*i);
		
		header()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(header(),SIGNAL(customContextMenuRequested(const QPoint & ) ),this,SLOT(showHeaderMenu( const QPoint& )));
		header_menu = new KMenu(this);
		header_menu->addTitle(i18n("Columns"));
		
		int idx = 0;
		foreach (QString col,columns)
		{
			QAction* act = header_menu->addAction(col);
			act->setCheckable(true);
			act->setChecked(true);
			column_idx_map[act] = idx++;
		}
		
		connect(header_menu,SIGNAL(triggered(QAction* )),this,SLOT(onHeaderMenuItemTriggered(QAction*)));
	}

	View::~View()
	{
	}

	void View::setGroup(Group* g)
	{
		group = g;
		update();
	}

	bool View::update()
	{
		Uint32 torrents = 0;
		Uint32 running = 0;
		// update items which are part of the current group
		// if they are not part of the current group, just hide them
		for (QMap<bt::TorrentInterface*,ViewItem*>::iterator i = items.begin();i != items.end();i++)
		{
			ViewItem* v = i.value();
			bt::TorrentInterface* ti = i.key();
			if (!group || (group && group->isMember(ti)))
			{
				if (v->isHidden())
					v->setHidden(false);

				v->update();
				torrents++;
				if (ti->getStats().running)
					running++;
			}
			else if (!v->isHidden())
				v->setHidden(true);
		}

		// update the caption
		if (num_running != running || num_torrents != torrents)
		{
			num_running = running;
			num_torrents = torrents;
			return true;
		}
		return false;
	}

	bool View::needToUpdateCaption()
	{
		Uint32 torrents = 0;
		Uint32 running = 0;
		for (QMap<bt::TorrentInterface*,ViewItem*>::iterator i = items.begin();i != items.end();i++)
		{
			ViewItem* v = i.value();
			bt::TorrentInterface* ti = i.key();
			if (!group || (group && group->isMember(ti)))
			{
				torrents++;
				if (ti->getStats().running)
					running++;
			}
		}

		if (num_running != running || num_torrents != torrents)
		{
			num_running = running;
			num_torrents = torrents;
			return true;
		}
		return false;
	}
	
	QString View::caption() const
	{
		return QString("%1 %2/%3").arg(group->groupName()).arg(num_running).arg(num_torrents);
	}


	void View::addTorrent(bt::TorrentInterface* ti)
	{
		ViewItem* v = new ViewItem(ti,this);
		items.insert(ti,v);
	}

	void View::removeTorrent(bt::TorrentInterface* ti)
	{
		ViewItem* v = items.value(ti);
		if (!v)
			return;

		items.remove(ti);
		delete v;
	}


	void View::startTorrents()
	{
		QList<QTreeWidgetItem *>  sel = selectedItems();
		foreach(QTreeWidgetItem* i,sel)
		{
			ViewItem* v = (ViewItem*)i;
			wantToStart(v->tc);
		}
	}

	void View::stopTorrents()
	{
		QList<QTreeWidgetItem *>  sel = selectedItems();
		foreach(QTreeWidgetItem* i,sel)
		{
			ViewItem* v = (ViewItem*)i;
			wantToStop(v->tc,true);
		}
	}

	void View::removeTorrents()
	{
		QList<QTreeWidgetItem *>  sel = selectedItems();
		foreach(QTreeWidgetItem* i,sel)
		{
			bool dummy = false;
			ViewItem* v = (ViewItem*)i;
			TorrentInterface* tc = v->tc;
			if (tc && !tc->isCheckingData(dummy))
			{	
				const TorrentStats & s = tc->getStats();
				bool data_to = false;
				if (s.bytes_left_to_download > 0)
				{
					QString msg = i18n("The torrent <b>%1</b> has not finished downloading, "
							"do you want to delete the incomplete data, too?",s.torrent_name);
					int ret = KMessageBox::questionYesNoCancel(
							this,
							msg,
							i18n("Remove Download"),
							KGuiItem(i18n("Delete Data")),
							KGuiItem(i18n("Keep Data")),
							KStandardGuiItem::cancel());
					if (ret == KMessageBox::Cancel)
						return;
					else if (ret == KMessageBox::Yes)
						data_to = true;
				}
				wantToRemove(tc,data_to);
			}
		}
	}

	void View::removeTorrentsAndData()
	{
		QList<QTreeWidgetItem *>  sel = selectedItems();
		if (sel.count() == 0)
			return;

		QString msg = i18n("You will lose all the downloaded data. Are you sure you want to do this?");
		if (KMessageBox::warningYesNo(this,msg, i18n("Remove Torrent"), KStandardGuiItem::remove(),KStandardGuiItem::cancel()) == KMessageBox::No)
			return;

		foreach(QTreeWidgetItem* i,sel)
		{
			bool dummy = false;
			ViewItem* v = (ViewItem*)i;
			TorrentInterface* tc = v->tc;
			if (tc && !tc->isCheckingData(dummy))
				wantToRemove(tc,true);
		}
	}

	void View::startAllTorrents()
	{
		foreach (bt::TorrentInterface* tc,items.keys())
		{
			wantToStart(tc);
		}
	}

	void View::stopAllTorrents()
	{
		foreach (bt::TorrentInterface* tc,items.keys())
		{
			wantToStop(tc,true);
		}
	}

	void View::queueTorrents()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			bool dummy;
			if (tc && !tc->isCheckingData(dummy))
				core->queue(tc);
		}
	}

	void View::addPeers()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() > 0)
		{
			AddPeersDlg dlg(sel[0],this);
			dlg.exec();
		}
	}

	void View::manualAnnounce()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			if (tc->getStats().running) 
				tc->updateTracker();
		}
	}

	void View::scrape()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			tc->scrapeTracker();
		}
	}

	void View::previewTorrents()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			if (tc->readyForPreview() && !tc->getStats().multi_file_torrent)
			{
				QFileInfo fi(tc->getTorDir()+"cache");                          
				new KRun(KUrl(fi.symLinkTarget()), 0, true, true);
			}
		}
	}

	void View::openDataDir()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			if (tc->getStats().multi_file_torrent)
				new KRun(KUrl(tc->getStats().output_path), 0, true, true);
			else
				new KRun(KUrl(tc->getDataDir()), 0, true, true);
		}
	}
	
	void View::openTorDir()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			new KRun(KUrl(tc->getTorDir()), 0, true, true);
		}
	}
	
	void View::removeFromGroup()
	{
		if (!group || group->isStandardGroup())
			return;

		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
			group->removeTorrent(tc);

		update();
	}
	
	void View::speedLimitsDlg()
	{
		SpeedLimitsDlg dlg(core,this);
		dlg.exec();
	}

	void View::toggleDHT()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			bool dummy;
			if (tc && !tc->isCheckingData(dummy))
			{
				bool on = tc->isFeatureEnabled(bt::DHT_FEATURE);
				tc->setFeatureEnabled(bt::DHT_FEATURE,!on);
			}
		}							
	}

	void View::togglePEX()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach (bt::TorrentInterface* tc,sel)
		{
			bool dummy;
			if (tc && !tc->isCheckingData(dummy))
			{
				bool on = tc->isFeatureEnabled(bt::UT_PEX_FEATURE);
				tc->setFeatureEnabled(bt::UT_PEX_FEATURE,!on);
			}
		}
	}

	void View::checkData()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() == 0)
			return;
		
		ScanDlg* dlg = new ScanDlg(core,false,this);
		dlg->show();
		dlg->execute(sel.front(),false);
	}

	void View::showMenu(const QPoint & pos)
	{
		menu->show(mapToGlobal(pos));
	}
	
	void View::showHeaderMenu(const QPoint& pos)
	{
		header_menu->popup(header()->mapToGlobal(pos));
	}

	void View::getSelection(QList<bt::TorrentInterface*> & sel)
	{
		QList<QTreeWidgetItem *> cur_sel =  selectedItems();
		foreach (QTreeWidgetItem* wi,cur_sel)
		{
			ViewItem* vi = (ViewItem*)wi;
			sel.append(vi->tc);
		}
	}

	void View::saveState(KSharedConfigPtr cfg,int idx)
	{
		KConfigGroup g = cfg->group(QString("View%1").arg(idx));
		QByteArray s = header()->saveState();
		g.writeEntry("state",s.toBase64());
	}


	void View::loadState(KSharedConfigPtr cfg,int idx)
	{
		KConfigGroup g = cfg->group(QString("View%1").arg(idx));
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
			header()->restoreState(s);
		
		QMap<QAction*,int>::iterator i = column_idx_map.begin();
		while (i != column_idx_map.end())
		{
			QAction* act = i.key();
			act->setChecked(!header()->isSectionHidden(i.value()));
			i++;
		}
	}

	bt::TorrentInterface* View::getCurrentTorrent()
	{
		ViewItem* vi = (ViewItem*)currentItem();
		return vi ? vi->tc : 0;
	}

	void View::onCurrentItemChanged(QTreeWidgetItem * current,QTreeWidgetItem * /*previous*/)
	{
		ViewItem* vi = (ViewItem*)current;
		currentTorrentChanged(this,vi ? vi->tc : 0);
	}

	void View::onHeaderMenuItemTriggered(QAction* act)
	{
		int idx = column_idx_map[act];
		if (act->isChecked())
			header()->showSection(idx);
		else
			header()->hideSection(idx);
	}
}

#include "view.moc"

