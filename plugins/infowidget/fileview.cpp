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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <QHeaderView>
#include <QItemSelectionModel>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kmenu.h>
#include <krun.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <util/bitset.h>
#include <util/functions.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <qfileinfo.h>
#include <util/log.h>
#include "fileview.h"
#include "iwfiletreemodel.h"
#include "iwfilelistmodel.h"
	
using namespace bt;

namespace kt
{

	FileView::FileView(QWidget *parent) : QTreeView(parent),curr_tc(0),model(0)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		setRootIsDecorated(false);
		setSortingEnabled(true);
		setAlternatingRowColors(true);
		setSelectionMode(QAbstractItemView::ExtendedSelection);
		setSelectionBehavior(QAbstractItemView::SelectRows);
		
		context_menu = new KMenu(this);
		open_action = context_menu->addAction(KIcon("document-open"),i18n("Open"),this,SLOT(open()));
		context_menu->addSeparator();
		download_first_action = context_menu->addAction(i18n("Download first"),this,SLOT(downloadFirst()));
		download_normal_action = context_menu->addAction(i18n("Download normally"),this,SLOT(downloadNormal()));
		download_last_action = context_menu->addAction(i18n("Download last"),this,SLOT(downloadLast()));
		context_menu->addSeparator();
		dnd_action = context_menu->addAction(i18n("Do Not Download"),this,SLOT(doNotDownload()));
		delete_action = context_menu->addAction(i18n("Delete File(s)"),this,SLOT(deleteFiles()));
		
		connect(this,SIGNAL(customContextMenuRequested(const QPoint & )),
				this,SLOT(showContextMenu(const QPoint& )));
		connect(this,SIGNAL(itemDoubleClicked(const QModelIndex & )),
				this,SLOT(onDoubleClicked(const QModelIndex & )));
		
		setEnabled(false);
		show_list_of_files = false;
	}


	FileView::~FileView()
	{}

	void FileView::changeTC(bt::TorrentInterface* tc,KSharedConfigPtr cfg)
	{
		if (tc == curr_tc)
			return;
	
		if (model)
		{
			saveState(cfg);
			if (curr_tc)
				expanded_state_map[curr_tc] = model->saveExpandedState(this);
		}
		setModel(0);
		delete model;
		model = 0;
		curr_tc = tc;
		setEnabled(tc != 0);
		if (tc)
		{
			connect(tc,SIGNAL(missingFilesMarkedDND( bt::TorrentInterface* )),
					this,SLOT(onMissingFileMarkedDND(bt::TorrentInterface*)));
			
			if (show_list_of_files)
				model = new IWFileListModel(tc,this);
			else 
				model = new IWFileTreeModel(tc,this);
			
			setModel(model);
			setRootIsDecorated(tc->getStats().multi_file_torrent);
			loadState(cfg);
			QMap<bt::TorrentInterface*,QByteArray>::iterator i = expanded_state_map.find(tc);
			if (i != expanded_state_map.end())
				model->loadExpandedState(this,i.value());
			else
				expandAll();
		}
		else
		{
			setModel(0);
			model = 0;
		}
	}
	
	void FileView::onMissingFileMarkedDND(bt::TorrentInterface* tc)
	{
		if (curr_tc == tc)
			model->missingFilesMarkedDND();
	}
		
	void FileView::showContextMenu(const QPoint & p)
	{
		const TorrentStats & s = curr_tc->getStats();
		
		QModelIndexList sel = selectionModel()->selectedRows();
		if (sel.count() == 0)
			return;
		
		if (sel.count() > 1)
		{
			download_first_action->setEnabled(true);
			download_normal_action->setEnabled(true);
			download_last_action->setEnabled(true);
			open_action->setEnabled(false);
			dnd_action->setEnabled(true);
			delete_action->setEnabled(true);
			context_menu->popup(mapToGlobal(p));
			return;
		}
	
		QModelIndex item = sel.front();
		bt::TorrentFileInterface* file = model->indexToFile(item);

		download_first_action->setEnabled(false);
		download_last_action->setEnabled(false);
		download_normal_action->setEnabled(false);
		dnd_action->setEnabled(false);
		delete_action->setEnabled(false);
		
		if (!s.multi_file_torrent)
		{
			open_action->setEnabled(true);
			preview_path = curr_tc->getStats().output_path;
		}
		else if (file)
		{
			if (!file->isNull())
			{
				open_action->setEnabled(true);
				preview_path = file->getPathOnDisk();
				
				download_first_action->setEnabled(file->getPriority() != FIRST_PRIORITY);
				download_normal_action->setEnabled(file->getPriority() != NORMAL_PRIORITY);
				download_last_action->setEnabled(file->getPriority() != LAST_PRIORITY);
				dnd_action->setEnabled(file->getPriority() != ONLY_SEED_PRIORITY);
				delete_action->setEnabled(file->getPriority() != EXCLUDED);
			}
			else
			{
				open_action->setEnabled(false);
			}
		}
		else
		{
			download_first_action->setEnabled(true);
			download_normal_action->setEnabled(true);
			download_last_action->setEnabled(true);
			dnd_action->setEnabled(true);
			delete_action->setEnabled(true);
			open_action->setEnabled(true);
			preview_path = curr_tc->getDataDir() + model->dirPath(item);
		}

		context_menu->popup(mapToGlobal(p));
	}
	
	void FileView::open()
	{
		new KRun(KUrl(preview_path), 0, true, true);
	}
	
	void FileView::changePriority(bt::Priority newpriority)
	{
		model->changePriority(selectionModel()->selectedRows(2),newpriority);
	}


	void FileView::downloadFirst()
	{
		changePriority(FIRST_PRIORITY);
	}

	void FileView::downloadLast()
	{
		changePriority(LAST_PRIORITY);
	}

	void FileView::downloadNormal()
	{
		changePriority(NORMAL_PRIORITY);
	}

	void FileView::doNotDownload()
	{
		changePriority(ONLY_SEED_PRIORITY);
	}

	void FileView::deleteFiles()
	{
		QModelIndexList sel = selectionModel()->selectedRows();
		Uint32 n = sel.count();
		if (n == 1) // single item can be a directory
		{
			if (!model->indexToFile(sel.front()))
				n++;
		} 
			
		QString msg = n > 1 ? i18n("You will lose all data in this file, are you sure you want to do this ?") :
				i18n("You will lose all data in these files, are you sure you want to do this ?");
					
		if (KMessageBox::warningYesNo(0,msg) == KMessageBox::Yes)
			changePriority(EXCLUDED);
	}
	
	void FileView::onDoubleClicked(const QModelIndex & index)
	{
		if (!curr_tc)
			return;
		
		const TorrentStats & s = curr_tc->getStats();
		
		if (s.multi_file_torrent)
		{
			bt::TorrentFileInterface* file = model->indexToFile(index);
			if (!file)
			{
				// directory
				new KRun(KUrl(curr_tc->getDataDir() + model->dirPath(index)), 0, true, true);
			}
			else
			{
				// file
				new KRun(KUrl(file->getPathOnDisk()), 0, true, true);
			}
		}
		else
		{
			new KRun(KUrl(curr_tc->getStats().output_path), 0, true, true);
		}
	}
	
	void FileView::saveState(KSharedConfigPtr cfg)
	{
		if (!model)
			return;
		
		KConfigGroup g = cfg->group("FileView");
		QByteArray s = header()->saveState();
		g.writeEntry("state",s.toBase64());
	}
	
	void FileView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("FileView");
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
			header()->restoreState(s);
	}

	void FileView::update()
	{
		if (model)
			model->update();
	}
	
	void FileView::onTorrentRemoved(bt::TorrentInterface* tc)
	{
		expanded_state_map.remove(tc);
	}
	
	void FileView::setShowListOfFiles(bool on,KSharedConfigPtr cfg)
	{
		if (show_list_of_files == on)
			return;
		
		show_list_of_files = on;
		if (!model || !curr_tc)
			return;
		
		saveState(cfg);
		expanded_state_map[curr_tc] = model->saveExpandedState(this);
		
		setModel(0);
		delete model;
		model = 0;
			
		if (show_list_of_files)
			model = new IWFileListModel(curr_tc,this);
		else 
			model = new IWFileTreeModel(curr_tc,this);
			
		setModel(model);
		setRootIsDecorated(curr_tc->getStats().multi_file_torrent);
		loadState(cfg);
		QMap<bt::TorrentInterface*,QByteArray>::iterator i = expanded_state_map.find(curr_tc);
		if (i != expanded_state_map.end())
			model->loadExpandedState(this,i.value());
		else
			expandAll();
	}
}

#include "fileview.moc"
