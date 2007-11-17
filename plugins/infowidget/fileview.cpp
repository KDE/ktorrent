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
#include "iwfiletreeitem.h"
#include "iwfiletreediritem.h"
#include "fileview.h"
		
using namespace bt;

namespace kt
{

	FileView::FileView(QWidget *parent) : QTreeWidget(parent),curr_tc(0),multi_root(0)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		setRootIsDecorated(false);
		setSortingEnabled(true);
		setAlternatingRowColors(true);
		setSelectionMode(QAbstractItemView::ExtendedSelection);
		setSelectionBehavior(QAbstractItemView::SelectRows);

		QStringList columns;
		columns << i18n( "File" ) <<  i18n( "Size" ) <<  i18n( "Download" ) << i18n( "Preview" ) << i18n( "% Complete" );
		setHeaderLabels(columns);
		
		context_menu = new KMenu(this);
		open_action = context_menu->addAction(KIcon("document-open"),i18n("Open"),this,SLOT(open()));
		context_menu->addSeparator();
		download_first_action = context_menu->addAction(i18n("Download first"),this,SLOT(downloadFirst()));
		download_normal_action = context_menu->addAction(i18n("Download normally"),this,SLOT(downloadLast()));
		download_last_action = context_menu->addAction(i18n("Download last"),this,SLOT(downloadLast()));
		context_menu->addSeparator();
		dnd_action = context_menu->addAction(i18n("Do Not Download"),this,SLOT(doNotDownload()));
		delete_action = context_menu->addAction(i18n("Delete File(s)"),this,SLOT(deleteFiles()));
		
		
		connect(this,SIGNAL(customContextMenuRequested(const QPoint & )),
				this,SLOT(showContextMenu(const QPoint& )));
		connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem* , int)),
				this,SLOT(onDoubleClicked(QTreeWidgetItem* , int)));
		
		setEnabled(false);
	}


	FileView::~FileView()
	{}
	
	void FileView::fillFileTree()
	{
		multi_root = 0;
		clear();
	
		if (!curr_tc)
			return;
	
		if (curr_tc->getStats().multi_file_torrent)
		{
			IWFileTreeDirItem* root = new IWFileTreeDirItem(
					this,curr_tc->getStats().torrent_name,0);
			
			for (Uint32 i = 0;i < curr_tc->getNumFiles();i++)
			{
				TorrentFileInterface & file = curr_tc->getTorrentFile(i);
				root->insert(file.getPath(),file,kt::KEEP_FILES);
			}
			root->setExpanded(true);
			setRootIsDecorated(true);
			multi_root = root;
			multi_root->updatePriorityInformation(curr_tc);
			multi_root->updatePercentageInformation();
			multi_root->updatePreviewInformation(curr_tc);
		}
		else
		{
			const TorrentStats & s = curr_tc->getStats();
			this->setRootIsDecorated(false);
			QTreeWidgetItem* item = new QTreeWidgetItem(this);
			item->setText(0,s.torrent_name);
			item->setText(1,BytesToString(s.total_bytes));
			item->setIcon(0,KIcon(KMimeType::iconNameForUrl(s.torrent_name)));
		}
	}

	void FileView::changeTC(bt::TorrentInterface* tc)
	{
		if (tc == curr_tc)
			return;
	
		curr_tc = tc;
		fillFileTree();
		setEnabled(tc != 0);
		if (tc)
			connect(tc,SIGNAL(missingFilesMarkedDND( bt::TorrentInterface* )),
					this,SLOT(refreshFileTree( bt::TorrentInterface* )));
	}
	
	void FileView::update()
	{
		if (!curr_tc)
			return;
		
		if (isVisible())
		{
			readyPreview();
			readyPercentage();
		}
	}
	
	void FileView::readyPercentage()
	{
		if (curr_tc && !curr_tc->getStats().multi_file_torrent)
		{
			QTreeWidgetItem* item = topLevelItem(0);
			if (!item)
				return;
						
			const BitSet & bs = curr_tc->downloadedChunksBitSet();
			Uint32 total = bs.getNumBits();
			Uint32 on = bs.numOnBits();			
			double percent = 100.0 * ((double)on/(double)total);
			if (percent < 0.0)
				percent = 0.0;
			else if (percent > 100.0)
				percent = 100.0;
			KLocale* loc = KGlobal::locale();
			item->setText(4,i18n("%1 %",loc->formatNumber(percent,2)));
		}
	}

	void FileView::readyPreview()
	{
		if (curr_tc && !curr_tc->getStats().multi_file_torrent)
		{
			QTreeWidgetItem* item = topLevelItem(0);
			if (!item)
				return;
			
			if (IsMultimediaFile(curr_tc->getStats().output_path))
			{
				if ( curr_tc->readyForPreview() )
					item->setText(3, i18n("Available"));
				else
					item->setText(3, i18n("Pending"));
			}
			else
				item->setText(3, i18n("No"));
			
		}
	}
	
	void FileView::showContextMenu(const QPoint & p)
	{
		const TorrentStats & s = curr_tc->getStats();
		// don't show a menu if item is 0 or if it is a directory
		
		QList<QTreeWidgetItem*> sel = selectedItems();
		if (sel.count() > 1)
		{
			download_first_action->setEnabled(true);
			download_normal_action->setEnabled(true);
			download_last_action->setEnabled(true);
			open_action->setEnabled(false);
			dnd_action->setEnabled(true);
			delete_action->setEnabled(true);
			context_menu->popup(mapToGlobal(p));
		}
		else if (sel.count() == 0)
			return;

		QTreeWidgetItem* item = sel.front();

		download_first_action->setEnabled(false);
		download_last_action->setEnabled(false);
		download_normal_action->setEnabled(false);
		dnd_action->setEnabled(false);
		delete_action->setEnabled(false);

		if (s.multi_file_torrent && item->childCount() == 0)
		{
			bt::TorrentFileInterface & file = ((FileTreeItem*)item)->getTorrentFile();
			if (!file.isNull())
			{
				open_action->setEnabled(true);
				preview_path = "cache" + bt::DirSeparator() + file.getPath();
				
				download_first_action->setEnabled(file.getPriority() != FIRST_PRIORITY);
				download_normal_action->setEnabled(file.getPriority() != NORMAL_PRIORITY);
				download_last_action->setEnabled(file.getPriority() != LAST_PRIORITY);
				dnd_action->setEnabled(file.getPriority() != ONLY_SEED_PRIORITY);
				delete_action->setEnabled(file.getPriority() != EXCLUDED);
			}
			else
			{
				open_action->setEnabled(false);
			}
		}
		else
		{
			bool val = item->childCount() != 0;
			download_first_action->setEnabled(val);
			download_normal_action->setEnabled(val);
			download_last_action->setEnabled(val);
			dnd_action->setEnabled(val);
			delete_action->setEnabled(val);

			open_action->setEnabled(true);
			if (s.multi_file_torrent)
			{
				FileTreeDirItem* dir = ((FileTreeDirItem*)item);
				preview_path = "cache" + dir->getPath();
			}
			else
			{
				preview_path = "cache";
			}
		}

		context_menu->popup(mapToGlobal(p));
	}
	
	void FileView::open()
	{
		new KRun(KUrl(curr_tc->getTorDir() + preview_path), 0, true, true);
	}
	
	void FileView::changePriority(bt::Priority newpriority)
	{
		foreach (QTreeWidgetItem* item,selectedItems())
			changePriority(item,newpriority);

		multi_root->updatePriorityInformation(curr_tc);
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
		QList<QTreeWidgetItem*> sel = selectedItems();
		Uint32 n = sel.count();
		if (n == 1) // single item can be a directory
		{ 
			// the number of the beast > 1
			n = (*sel.begin())->childCount() == 0 ? 1 : 666;
		} 
			
		QString msg = n > 1 ? i18n("You will lose all data in this file, are you sure you want to do this ?") :
				i18n("You will lose all data in these files, are you sure you want to do this ?");
					
		if (KMessageBox::warningYesNo(0,msg) == KMessageBox::Yes)
			changePriority(EXCLUDED);
	}

	void FileView::changePriority(QTreeWidgetItem* item, Priority newpriority)
	{	
		if(item->childCount() == 0)
		{
			FileTreeItem* fti = (FileTreeItem*)item;
			if (newpriority == EXCLUDED)
			{
				fti->setChecked(false,false);
			}
			else if (newpriority == ONLY_SEED_PRIORITY)
			{
				fti->setChecked(false,true);
			}
			else 
			{
				if (!fti->isOn())
					fti->setChecked(true,true);
				fti->getTorrentFile().setPriority(newpriority);
			}
			return;
		}

		for (int i = 0;i < item->childCount();i++)
			changePriority(item->child(i), newpriority);
	}
	
	void FileView::refreshFileTree(bt::TorrentInterface* tc)
	{
		if (!tc || curr_tc != tc)
			return;
		
		if (multi_root)
			multi_root->updateDNDInformation();
	}
	
	void FileView::onDoubleClicked(QTreeWidgetItem* item,int)
	{
		if (!curr_tc)
			return;
		
		const TorrentStats & s = curr_tc->getStats();
		
		if (s.multi_file_torrent)
		{
			if (item->childCount() == 0)
			{
				// file
				FileTreeItem* file = (FileTreeItem*)item;
				QString path = "cache" + bt::DirSeparator() + file->getTorrentFile().getPath();
				new KRun(KUrl(curr_tc->getTorDir() + path), 0, true, true);
			}
			else
			{
				// directory
				FileTreeDirItem* dir = ((FileTreeDirItem*)item);
				new KRun(KUrl(curr_tc->getTorDir() + "cache" + dir->getPath()), 0, true, true);
			}
		}
		else
		{
			QFileInfo fi(curr_tc->getTorDir()+"cache");
			new KRun(KUrl(fi.readLink()), 0, true, true);
		}
	}
	
	void FileView::saveState(KSharedConfigPtr cfg)
	{
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

}

#include "fileview.moc"
