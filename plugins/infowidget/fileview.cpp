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
#include "fileview.h"

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
#include <kfiledialog.h>
#include <util/bitset.h>
#include <util/error.h>
#include <util/functions.h>
#include <util/treefiltermodel.h>
#include <util/hintlineedit.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <qfileinfo.h>
#include <util/log.h>
#include <util/timer.h>
#include "iwfiletreemodel.h"
#include "iwfilelistmodel.h"
#include <QHBoxLayout>
#include <QToolBar>
	
using namespace bt;

namespace kt
{

	FileView::FileView(QWidget *parent) : QWidget(parent),curr_tc(0),model(0),show_list_of_files(false)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setMargin(0);
		layout->setSpacing(0);
		QVBoxLayout* vbox = new QVBoxLayout();
		vbox->setMargin(0);
		vbox->setSpacing(0);
		view = new QTreeView(this);
		toolbar = new QToolBar(this);
		toolbar->setOrientation(Qt::Vertical);
		toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		layout->addWidget(toolbar);
		
		filter = new HintLineEdit(this);
		filter->setHintText(i18n("Filter"));
		filter->setClearButtonShown(true);
		filter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		connect(filter,SIGNAL(textChanged(QString)),this,SLOT(setFilter(QString)));
		filter->hide();
		vbox->addWidget(filter);
		vbox->addWidget(view);
		layout->addItem(vbox);
		
		view->setContextMenuPolicy(Qt::CustomContextMenu);
		view->setRootIsDecorated(false);
		view->setSortingEnabled(true);
		view->setAlternatingRowColors(true);
		view->setSelectionMode(QAbstractItemView::ExtendedSelection);
		view->setSelectionBehavior(QAbstractItemView::SelectRows);
		view->setUniformRowHeights(true);
		
		proxy_model = new TreeFilterModel(this);
		proxy_model->setSortRole(Qt::UserRole);
		if (show_list_of_files)
			model = new IWFileListModel(0,this);
		else 
			model = new IWFileTreeModel(0,this);
		proxy_model->setSourceModel(model);
		view->setModel(proxy_model);
		
		setupActions();
		
		connect(view,SIGNAL(customContextMenuRequested(const QPoint & )),
				this,SLOT(showContextMenu(const QPoint& )));
		connect(view,SIGNAL(doubleClicked(const QModelIndex & )),
				this,SLOT(onDoubleClicked(const QModelIndex & )));
		
		setEnabled(false);
		
	}
	
	FileView::~FileView()
	{}
	
	void FileView::setupActions()
	{
		context_menu = new KMenu(this);
		open_action = context_menu->addAction(KIcon("document-open"),i18nc("Open file", "Open"),this,SLOT(open()));
		context_menu->addSeparator();
		download_first_action = context_menu->addAction(i18n("Download first"),this,SLOT(downloadFirst()));
		download_normal_action = context_menu->addAction(i18n("Download normally"),this,SLOT(downloadNormal()));
		download_last_action = context_menu->addAction(i18n("Download last"),this,SLOT(downloadLast()));
		context_menu->addSeparator();
		dnd_action = context_menu->addAction(i18n("Do Not Download"),this,SLOT(doNotDownload()));
		delete_action = context_menu->addAction(i18n("Delete File(s)"),this,SLOT(deleteFiles()));
		context_menu->addSeparator();
		move_files_action = context_menu->addAction(i18n("Move File"),this,SLOT(moveFiles()));
		context_menu->addSeparator();
		collapse_action = context_menu->addAction(i18n("Collapse Folder Tree"),this,SLOT(collapseTree()));
		expand_action = context_menu->addAction(i18n("Expand Folder Tree"),this,SLOT(expandTree()));
		
		
		QActionGroup* ag = new QActionGroup(this);
		show_tree_action = new QAction(KIcon("view-list-tree"),i18n("File Tree"),this);
		connect(show_tree_action,SIGNAL(triggered(bool)),this,SLOT(showTree()));
		show_list_action = new QAction(KIcon("view-list-text"),i18n("File List"),this);
		connect(show_list_action,SIGNAL(triggered(bool)),this,SLOT(showList()));
		ag->addAction(show_list_action);
		ag->addAction(show_tree_action);
		ag->setExclusive(true);
		show_list_action->setCheckable(true);
		show_tree_action->setCheckable(true);
		toolbar->addAction(show_tree_action);
		toolbar->addAction(show_list_action);
		
		show_filter_action = new QAction(KIcon("view-filter"),i18n("Show Filter"),this);
		show_filter_action->setCheckable(true);
		connect(show_filter_action,SIGNAL(toggled(bool)),filter,SLOT(setShown(bool)));
		toolbar->addAction(show_filter_action);
	}

	void FileView::changeTC(bt::TorrentInterface* tc)
	{
		if (tc == curr_tc)
			return;
	
		if (curr_tc)
			expanded_state_map[curr_tc] = model->saveExpandedState(proxy_model,view);
		
		curr_tc = tc;
		setEnabled(tc != 0);
		model->changeTorrent(tc);
		if (tc)
		{
			connect(tc,SIGNAL(missingFilesMarkedDND( bt::TorrentInterface* )),
					this,SLOT(onMissingFileMarkedDND(bt::TorrentInterface*)));
			
			view->setRootIsDecorated(!show_list_of_files && tc->getStats().multi_file_torrent);
			if (!show_list_of_files)
			{
				QMap<bt::TorrentInterface*,QByteArray>::iterator i = expanded_state_map.find(tc);
				if (i != expanded_state_map.end())
					model->loadExpandedState(proxy_model,view,i.value());
				else
					view->expandAll();
			}
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
		
		QModelIndexList sel = view->selectionModel()->selectedRows();
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
			context_menu->popup(view->viewport()->mapToGlobal(p));
			move_files_action->setEnabled(true);
			collapse_action->setEnabled(!show_list_of_files);
			expand_action->setEnabled(!show_list_of_files);
			return;
		}
	
		QModelIndex item = proxy_model->mapToSource(sel.front());
		bt::TorrentFileInterface* file = model->indexToFile(item);

		download_first_action->setEnabled(false);
		download_last_action->setEnabled(false);
		download_normal_action->setEnabled(false);
		dnd_action->setEnabled(false);
		delete_action->setEnabled(false);
		
		if (!s.multi_file_torrent)
		{
			open_action->setEnabled(true);
			move_files_action->setEnabled(true);
			preview_path = curr_tc->getStats().output_path;
			collapse_action->setEnabled(false);
			expand_action->setEnabled(false);
		}
		else if (file)
		{
			move_files_action->setEnabled(true);
			collapse_action->setEnabled(false);
			expand_action->setEnabled(false);
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
			move_files_action->setEnabled(false);
			download_first_action->setEnabled(true);
			download_normal_action->setEnabled(true);
			download_last_action->setEnabled(true);
			dnd_action->setEnabled(true);
			delete_action->setEnabled(true);
			open_action->setEnabled(true);
			preview_path = curr_tc->getDataDir() + model->dirPath(item);
			collapse_action->setEnabled(!show_list_of_files);
			expand_action->setEnabled(!show_list_of_files);
		}

		context_menu->popup(view->viewport()->mapToGlobal(p));
	}
	
	void FileView::open()
	{
		new KRun(KUrl(preview_path), 0, 0, true, true);
	}
	
	void FileView::changePriority(bt::Priority newpriority)
	{
		QModelIndexList sel = view->selectionModel()->selectedRows(2);
		for (QModelIndexList::iterator i = sel.begin();i != sel.end();i++)
			*i = proxy_model->mapToSource(*i);
		
		model->changePriority(sel,newpriority);
		proxy_model->invalidate();
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
		QModelIndexList sel = view->selectionModel()->selectedRows();
		Uint32 n = sel.count();
		if (n == 1) // single item can be a directory
		{
			if (!model->indexToFile(proxy_model->mapToSource(sel.front())))
				n++;
		} 
			
		QString msg = i18np("You will lose all data in this file, are you sure you want to do this?",
				    "You will lose all data in these files, are you sure you want to do this?", n);
					
		if (KMessageBox::warningYesNo(0,msg) == KMessageBox::Yes)
			changePriority(EXCLUDED);
	}
	
	void FileView::moveFiles()
	{
		if (curr_tc->getStats().multi_file_torrent)
		{
			QModelIndexList sel = view->selectionModel()->selectedRows();
			QMap<bt::TorrentFileInterface*,QString> moves;
			
			QString dir = KFileDialog::getExistingDirectory(KUrl("kfiledialog:///saveTorrentData"),
					this,i18n("Select a directory to move the data to."));
			if (dir.isNull())
				return;
			
			foreach (const QModelIndex &idx,sel)
			{
				bt::TorrentFileInterface* tfi = model->indexToFile(proxy_model->mapToSource(idx));
				if (!tfi)
					continue;
			
				moves.insert(tfi,dir);
			}
			
			if (moves.count() > 0)
			{
				curr_tc->moveTorrentFiles(moves);
			}
		}
		else
		{
			QString dir = KFileDialog::getExistingDirectory(KUrl("kfiledialog:///saveTorrentData"),
					this,i18n("Select a directory to move the data to."));
			if (dir.isNull())
				return;
		
			curr_tc->changeOutputDir(dir,bt::TorrentInterface::MOVE_FILES);
		}
	}
	
	void FileView::expandCollapseTree(const QModelIndex& idx, bool expand) 
	{
		int rowCount = proxy_model->rowCount(idx);
		for (int i = 0; i < rowCount; i++) 
		{
			const QModelIndex& ridx = proxy_model->index(i, 0, idx);
			if (proxy_model->hasChildren(ridx))
				expandCollapseTree(ridx, expand);
		}
		view->setExpanded(idx, expand);
	}

	void FileView::expandCollapseSelected(bool expand) 
	{
		QModelIndexList sel = view->selectionModel()->selectedRows();
		for (QModelIndexList::iterator i = sel.begin(); i != sel.end(); i++) 
		{
			if (proxy_model->hasChildren(*i))
				expandCollapseTree(*i, expand);
		}
	}

	void FileView::collapseTree() 
	{
		expandCollapseSelected(false);
	}

	void FileView::expandTree() 
	{
		expandCollapseSelected(true);
	}

	void FileView::onDoubleClicked(const QModelIndex & index)
	{
		if (!curr_tc)
			return;
		
		const TorrentStats & s = curr_tc->getStats();
		
		if (s.multi_file_torrent)
		{
			bt::TorrentFileInterface* file = model->indexToFile(proxy_model->mapToSource(index));
			if (!file)
			{
				// directory
				new KRun(KUrl(curr_tc->getDataDir() + model->dirPath(proxy_model->mapToSource(index))), 0, 0, true, true);
			}
			else
			{
				// file
				new KRun(KUrl(file->getPathOnDisk()), 0, 0, true, true);
			}
		}
		else
		{
			new KRun(KUrl(curr_tc->getStats().output_path), 0, 0, true, true);
		}
	}
	
	void FileView::saveState(KSharedConfigPtr cfg)
	{
		if (!model)
			return;
		
		KConfigGroup g = cfg->group("FileView");
		QByteArray s = view->header()->saveState();
		g.writeEntry("state",s.toBase64());
		g.writeEntry("show_list_of_files",show_list_of_files);
	}
	
	void FileView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("FileView");
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
		{
			QHeaderView* v = view->header();
			v->restoreState(s);
			view->sortByColumn(v->sortIndicatorSection(),v->sortIndicatorOrder());
		}
		
		bool show_list = g.readEntry("show_list_of_files",false);
		if (show_list_of_files != show_list)
			setShowListOfFiles(show_list);
		
		show_list_action->setChecked(show_list);
		show_tree_action->setChecked(!show_list);
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
	
	void FileView::setShowListOfFiles(bool on)
	{
		if (show_list_of_files == on)
			return;
		
		QByteArray header_state = view->header()->saveState();
		show_list_of_files = on;
		if (!curr_tc)
		{
			// no torrent, but still need to change the model
			proxy_model->setSourceModel(0);
			delete model;
			if (show_list_of_files)
				model = new IWFileListModel(0,this);
			else 
				model = new IWFileTreeModel(0,this);
			proxy_model->setSourceModel(model);
			view->header()->restoreState(header_state);
			return;
		}
		
		if (on)
			expanded_state_map[curr_tc] = model->saveExpandedState(proxy_model,view);
		
		proxy_model->setSourceModel(0);
		delete model;
		model = 0;
			
		if (show_list_of_files)
			model = new IWFileListModel(curr_tc,this);
		else 
			model = new IWFileTreeModel(curr_tc,this);
			
		proxy_model->setSourceModel(model);
		view->setRootIsDecorated(!show_list_of_files && curr_tc->getStats().multi_file_torrent);
		view->header()->restoreState(header_state);
		
		if (!on)
		{
			QMap<bt::TorrentInterface*,QByteArray>::iterator i = expanded_state_map.find(curr_tc);
			if (i != expanded_state_map.end())
				model->loadExpandedState(proxy_model,view,i.value());
			else
				view->expandAll();
		}
		
		collapse_action->setEnabled(!show_list_of_files);
		expand_action->setEnabled(!show_list_of_files);
	}
	
	void FileView::showTree()
	{
		if (show_list_of_files)
			setShowListOfFiles(false);
	}
	
	void FileView::showList()
	{
		if (!show_list_of_files)
			setShowListOfFiles(true);
	}
	
	void FileView::filePercentageChanged(bt::TorrentFileInterface* file,float percentage)
	{
		if (model)
			model->filePercentageChanged(file,percentage);
	}
	
	void FileView::filePreviewChanged(bt::TorrentFileInterface* file,bool preview)
	{
		if (model)
			model->filePreviewChanged(file,preview);
	}
	
	
	void FileView::setFilter(const QString& f)
	{
		Q_UNUSED(f);
		proxy_model->setFilterFixedString(filter->typedText());
	}


}

#include "fileview.moc"
