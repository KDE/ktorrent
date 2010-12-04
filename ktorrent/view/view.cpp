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
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QSortFilterProxyModel>
#include <krun.h>
#include <kmenu.h>
#include <klocale.h>
#include <ksharedconfig.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <torrent/jobqueue.h>
#include <util/functions.h>
#include <util/log.h>
#include <interfaces/functions.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include "view.h"
#include "core.h"
#include "viewmodel.h"
#include "dialogs/speedlimitsdlg.h"
#include "dialogs/addpeersdlg.h"
#include "viewselectionmodel.h"
#include "viewdelegate.h"
#include "propertiesextender.h"

using namespace bt;

namespace kt
{
	
	View::View(Core* core,QWidget* parent) 
		: QTreeView(parent),core(core),group(0),num_torrents(0),num_running(0),model(0)
	{
		model = new ViewModel(core,this);
		selection_model = new ViewSelectionModel(model,this);
		
		setContextMenuPolicy(Qt::CustomContextMenu);
		setRootIsDecorated(false);
		setSortingEnabled(true);
		setAlternatingRowColors(true);
		setDragEnabled(true);
		setSelectionMode(QAbstractItemView::ExtendedSelection);
		setSelectionBehavior(QAbstractItemView::SelectRows);
		setAcceptDrops(true);
		setDragDropMode(DragDrop);
		setUniformRowHeights(true);
		
		connect(this,SIGNAL(customContextMenuRequested(const QPoint & ) ),this,SLOT(showMenu( const QPoint& )));
	
		header()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(header(),SIGNAL(customContextMenuRequested(const QPoint & ) ),this,SLOT(showHeaderMenu( const QPoint& )));
		header_menu = new KMenu(this);
		header_menu->addTitle(i18n("Columns"));
	
		for (int i = 0;i < model->columnCount(QModelIndex());i++)
		{
			QString col = model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString();
			QAction* act = header_menu->addAction(col);
			act->setCheckable(true);
			act->setChecked(true);
			column_idx_map[act] = i;
			column_action_list.append(act);
		}
		
		connect(header_menu,SIGNAL(triggered(QAction* )),this,SLOT(onHeaderMenuItemTriggered(QAction*)));
		
		setModel(model);
		setSelectionModel(selection_model);
		connect(selectionModel(),SIGNAL(currentChanged(const QModelIndex &,const QModelIndex &)),
				this,SLOT(onCurrentItemChanged(const QModelIndex&, const QModelIndex&)));
		connect(selectionModel(),SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection)),
				this,SLOT(onSelectionChanged(const QItemSelection &,const QItemSelection)));
		connect(model,SIGNAL(sorted()),selection_model,SLOT(sorted()));
		connect(this,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onDoubleClicked(QModelIndex)));
		
		delegate = new ViewDelegate(core,model,this);
		setItemDelegate(delegate);
	}

	View::~View()
	{
		delegate->contractAll();
	}
	
	void View::setupDefaultColumns()
	{
		int idx = 0;
		foreach (QAction* act,column_action_list)
		{
			bool ret = true;
			if (group->groupFlags() == Group::DOWNLOADS_ONLY_GROUP)
				ret = model->defaultColumnForDownload(idx);
			else if (group->groupFlags() == Group::UPLOADS_ONLY_GROUP)
				ret = model->defaultColumnForUpload(idx); 
			
			header()->setSectionHidden(idx,!ret);
			act->setChecked(ret);
			idx++;
		}
	}

	void View::setGroup(Group* g)
	{
		group = g;
		model->setGroup(group);
		update();
		needToUpdateCaption();
		selectionModel()->clear();
	}

	void View::update()
	{
		if (!uniformRowHeights() && !delegate->hasExtenders())
			setUniformRowHeights(true);
		model->update(delegate);
	}

	bool View::needToUpdateCaption()
	{
		Uint32 torrents = 0;
		Uint32 running = 0;
		QList<bt::TorrentInterface*> all;
		model->allTorrents(all);
		foreach (bt::TorrentInterface* ti,all)
		{
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
	
	QString View::caption(bool full) const
	{
		QString name = group->groupName();
		if (full)
			return QString("%1 %2/%3").arg(name).arg(num_running).arg(num_torrents);
		
		if (name.count() > 35)
			name = name.left(32) + "...";
		return QString("%1 %2/%3").arg(name).arg(num_running).arg(num_torrents);
	}

	void View::startTorrents()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() > 0)
			core->start(sel);
	}

	void View::stopTorrents()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() > 0)
			core->stop(sel);
	}

	void View::pauseTorrents()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() > 0)
			core->pause(sel);
	}
	
	void View::removeTorrents()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach(bt::TorrentInterface* tc,sel)
		{
			if (tc && !tc->getJobQueue()->runningJobs())
			{	
				const TorrentStats & s = tc->getStats();
				bool data_to = false;
				if (!s.completed)
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
				core->remove(tc,data_to);
			}
		}
	}

	void View::removeTorrentsAndData()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() == 0)
			return;

		QString msg = i18n("You will lose all the downloaded data. Are you sure you want to do this?");
		if (KMessageBox::warningYesNo(this,msg, i18n("Remove Torrent"), KStandardGuiItem::remove(),KStandardGuiItem::cancel()) == KMessageBox::No)
			return;

		core->remove(sel,true);
	}

	void View::startAllTorrents()
	{
		QList<bt::TorrentInterface*> all;
		model->allTorrents(all);
		core->start(all);
	}

	void View::stopAllTorrents()
	{
		QList<bt::TorrentInterface*> all;
		model->allTorrents(all);
		core->stop(all);
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
		foreach(bt::TorrentInterface* tc,sel)
		{
			if (tc->getStats().running) 
				tc->updateTracker();
		}
	}

	void View::scrape()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach(bt::TorrentInterface* tc,sel)
		{
			tc->scrapeTracker();
		}
	}

	void View::previewTorrents()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach(bt::TorrentInterface* tc,sel)
		{
			if (tc->readyForPreview() && !tc->getStats().multi_file_torrent)
			{
				new KRun(KUrl(tc->getStats().output_path), 0, 0, true, true);
			}
		}
	}

	void View::openDataDir()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach(bt::TorrentInterface* tc,sel)
		{
			if (tc->getStats().multi_file_torrent)
				new KRun(KUrl(tc->getStats().output_path), 0, 0, true, true);
			else
				new KRun(KUrl(tc->getDataDir()), 0, 0, true, true);
		}
	}
	
	void View::openTorDir()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach(bt::TorrentInterface* tc,sel)
		{
			new KRun(KUrl(tc->getTorDir()), 0, 0, true, true);
		}
	}
	
	void View::moveData()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() == 0)
			return;
		
		QString dir = KFileDialog::getExistingDirectory(KUrl("kfiledialog:///saveTorrentData"),this,i18n("Select a directory to move the data to."));
		if (dir.isNull())
			return;
		
		foreach(bt::TorrentInterface* tc,sel)
		{
			tc->changeOutputDir(dir,bt::TorrentInterface::MOVE_FILES);
		}
	}

	void View::showProperties()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() == 0)
			return;

		foreach(bt::TorrentInterface *tc,sel)
		{
			delegate->extend(tc,new PropertiesExtender(tc,0));
		}
	}

	void View::removeFromGroup()
	{
		if (!group || group->isStandardGroup())
			return;

		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		foreach(bt::TorrentInterface* tc,sel)
			group->removeTorrent(tc);
		core->getGroupManager()->saveGroups();
		update();
		num_running++; // set these wrong so that the caption is updated on the next update
		num_torrents++;
	}

	void View::renameTorrent()
	{
		QModelIndexList indices = selectionModel()->selectedRows();
		if (indices.count() == 0)
			return;
		
		QModelIndex idx = indices.front();
		QTreeView::edit(model->index(idx.row(),0));
		editingItem(true);
	}
	
	void View::checkData()
	{
		QList<bt::TorrentInterface*> sel;
		getSelection(sel);
		if (sel.count() > 0)
		{
			core->doDataCheck(sel.front());
			core->startUpdateTimer(); // make sure update timer of core is running
		}
	}

	void View::showMenu(const QPoint & pos)
	{
		showMenu(this,viewport()->mapToGlobal(pos));
	}
	
	void View::showHeaderMenu(const QPoint& pos)
	{
		header_menu->popup(header()->mapToGlobal(pos));
	}

	void View::getSelection(QList<bt::TorrentInterface*> & sel)
	{
		QModelIndexList indices = selectionModel()->selectedRows();
		model->torrentsFromIndexList(indices,sel);
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
		{
			QHeaderView* v = header();
			v->restoreState(s);
			sortByColumn(v->sortIndicatorSection(),v->sortIndicatorOrder());
			model->sort(v->sortIndicatorSection(),v->sortIndicatorOrder());
		}
		
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
		return model->torrentFromIndex(selectionModel()->currentIndex());
	}

	void View::onCurrentItemChanged(const QModelIndex & current,const QModelIndex & /*previous*/)
	{
		//Out(SYS_GEN|LOG_DEBUG) << "onCurrentItemChanged " << current.row() << endl;
		bt::TorrentInterface* tc = model->torrentFromIndex(current);
		currentTorrentChanged(this,tc);
	}

	void View::onHeaderMenuItemTriggered(QAction* act)
	{
		int idx = column_idx_map[act];
		if (act->isChecked())
			header()->showSection(idx);
		else
			header()->hideSection(idx);
	}
	
	void View::onSelectionChanged(const QItemSelection & /*selected*/,const QItemSelection & /*deselected*/)
	{
		torrentSelectionChanged(this);
	}
	
	
	QList<QAction*> View::columnActionList() const
	{
		return column_action_list;
	}

	void View::closeEditor(QWidget* editor,QAbstractItemDelegate::EndEditHint hint)
	{
		QTreeView::closeEditor(editor,hint);
		editingItem(false);
		setFocus();
	}
	
	bool View::edit(const QModelIndex & index,EditTrigger trigger,QEvent* event)
	{
		bool ret = QTreeView::edit(index,trigger,event);
		if (ret)
			editingItem(true);
		
		return ret;
	}

	void View::onDoubleClicked(const QModelIndex& index)
	{
		if (index.column() == 0) // double clicking on column 0 will change the name of a torrent
			return;
		
		bt::TorrentInterface* tc = model->torrentFromIndex(index);
		if (tc)
		{
			if (tc->getStats().multi_file_torrent)
				new KRun(KUrl(tc->getStats().output_path), 0, 0, true, true);
			else
				new KRun(KUrl(tc->getDataDir()), 0, 0, true, true);
		}
	}
	
	void View::extend(TorrentInterface* tc, Extender* widget)
	{
		setUniformRowHeights(false);
		delegate->extend(tc,widget);
		if (group && !group->isMember(tc))
			delegate->hideExtender(tc);
	}

	
}

#include "view.moc"

