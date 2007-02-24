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
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <util/bitset.h>
#include <util/functions.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "functions.h"
#include "iwfiletreeitem.h"
#include "iwfiletreediritem.h"
#include "fileview.h"
		
using namespace bt;

namespace kt
{

	FileView::FileView(QWidget *parent, const char *name)
			: KListView(parent, name),curr_tc(0),multi_root(0)
	{
		setFrameShape(QFrame::NoFrame);
		addColumn( i18n( "File" ) );
    	addColumn( i18n( "Size" ) );
    	addColumn( i18n( "Download" ) );
    	addColumn( i18n( "Preview" ) );
    	addColumn( i18n( "% Complete" ) );
		KIconLoader* iload = KGlobal::iconLoader();
		context_menu = new KPopupMenu(this);
		preview_id = context_menu->insertItem(iload->loadIconSet("frame_image",KIcon::Small),
											  i18n("Preview"));
	    context_menu->insertSeparator();
		first_id = context_menu->insertItem(i18n("Download First"));
		normal_id = context_menu->insertItem(i18n("Download Normally"));
		last_id = context_menu->insertItem(i18n("Download Last"));
		context_menu->insertSeparator();
		dnd_keep_id = context_menu->insertItem(i18n("Do Not Download"));
		dnd_throw_away_id = context_menu->insertItem(i18n("Delete File(s)"));
		
		
		context_menu->setItemEnabled(preview_id, false);
		context_menu->setItemEnabled(first_id, false);
		context_menu->setItemEnabled(normal_id, false);
		context_menu->setItemEnabled(last_id, false);
		context_menu->setItemEnabled(dnd_keep_id, false);
		context_menu->setItemEnabled(dnd_throw_away_id, false);

		connect(this,SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint& )),
				this,SLOT(showContextMenu(KListView*, QListViewItem*, const QPoint& )));
		connect(context_menu, SIGNAL ( activated ( int ) ), this, SLOT ( contextItem ( int ) ) );
		
		setEnabled(false);
		
		setSelectionMode(QListView::Extended);
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
					this,curr_tc->getStats().torrent_name);
			
			for (Uint32 i = 0;i < curr_tc->getNumFiles();i++)
			{
				TorrentFileInterface & file = curr_tc->getTorrentFile(i);
				root->insert(file.getPath(),file);
			}
			root->setOpen(true);
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
			KListViewItem* item = new KListViewItem(
					this,
					s.torrent_name,
					BytesToString(s.total_bytes));
	
			item->setPixmap(0,KMimeType::findByPath(s.torrent_name)->pixmap(KIcon::Small));
		}
	}

	void FileView::changeTC(kt::TorrentInterface* tc)
	{
		if (tc == curr_tc)
			return;
	
		curr_tc = tc;
		fillFileTree();
		setEnabled(tc != 0);
		if (tc)
			connect(tc,SIGNAL(missingFilesMarkedDND( kt::TorrentInterface* )),
					this,SLOT(refreshFileTree( kt::TorrentInterface* )));
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
			QListViewItemIterator it(this);
			if (!it.current())
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
			it.current()->setText(4,i18n("%1 %").arg(loc->formatNumber(percent,2)));
		}
	}

	void FileView::readyPreview()
	{
		if (curr_tc && !curr_tc->getStats().multi_file_torrent)
		{
			QListViewItemIterator it(this);
			if (!it.current())
				return;
			
			if (IsMultimediaFile(curr_tc->getStats().output_path))
			{
				if ( curr_tc->readyForPreview() )
					it.current()->setText(3, i18n("Available"));
				else
					it.current()->setText(3, i18n("Pending"));
			}
			else
				it.current()->setText(3, i18n("No"));
			
		}
	}
	
	void FileView::showContextMenu(KListView* ,QListViewItem*,const QPoint & p)
	{
		const TorrentStats & s = curr_tc->getStats();
		// don't show a menu if item is 0 or if it is a directory
		
		
		
		QPtrList<QListViewItem> sel = selectedItems();
		switch(sel.count())
		{
		case 0:
			return;
			break;
		case 1:
			break;
		default:
			context_menu->setItemEnabled(first_id, true);
			context_menu->setItemEnabled(normal_id, true);
			context_menu->setItemEnabled(last_id, true);
			context_menu->setItemEnabled(preview_id, false);
			context_menu->setItemEnabled(dnd_keep_id,true);
			context_menu->setItemEnabled(dnd_throw_away_id,true);
			context_menu->popup(p);
			return;
			break;
		}
		QListViewItem* item = sel.getFirst();

		context_menu->setItemEnabled(first_id, false);
		context_menu->setItemEnabled(normal_id, false);
		context_menu->setItemEnabled(last_id, false);
		if(s.multi_file_torrent && item->childCount() == 0)
		{
			kt::TorrentFileInterface & file = ((FileTreeItem*)item)->getTorrentFile();
			if (!file.isNull())
			{
				if(file.isMultimedia() && curr_tc->readyForPreview(file.getFirstChunk(), file.getFirstChunk()+1) )
				{
					context_menu->setItemEnabled(preview_id, true);
					this->preview_path = "cache" + bt::DirSeparator() + file.getPath();
				}
				else
					context_menu->setItemEnabled(preview_id, false);
		
				switch(file.getPriority())
				{
				case FIRST_PRIORITY:
					context_menu->setItemEnabled(normal_id, true);
					context_menu->setItemEnabled(last_id, true);
					context_menu->setItemEnabled(dnd_keep_id,true);
					context_menu->setItemEnabled(dnd_keep_id,true);
					context_menu->setItemEnabled(dnd_throw_away_id,true);
					break;
				case LAST_PRIORITY:
					context_menu->setItemEnabled(first_id, true);
					context_menu->setItemEnabled(normal_id, true);
					context_menu->setItemEnabled(dnd_keep_id,true);
					context_menu->setItemEnabled(dnd_keep_id,true);
					context_menu->setItemEnabled(dnd_throw_away_id,true);
					break;
				case EXCLUDED:
					context_menu->setItemEnabled(first_id,true);
					context_menu->setItemEnabled(normal_id,true);
					context_menu->setItemEnabled(last_id, true);
					context_menu->setItemEnabled(dnd_keep_id,true);
					context_menu->setItemEnabled(dnd_throw_away_id,false);
					break;
				case ONLY_SEED_PRIORITY:
					context_menu->setItemEnabled(first_id,true);
					context_menu->setItemEnabled(normal_id,true);
					context_menu->setItemEnabled(last_id, true);
					context_menu->setItemEnabled(dnd_keep_id,false);
					context_menu->setItemEnabled(dnd_throw_away_id,true);
					break;
				case PREVIEW_PRIORITY:
				default:
					context_menu->setItemEnabled(first_id, true);
					context_menu->setItemEnabled(normal_id,false);
					context_menu->setItemEnabled(last_id, true);
					context_menu->setItemEnabled(dnd_keep_id,true);
					context_menu->setItemEnabled(dnd_throw_away_id,true);
					break;
				}
			}
			else
			{
				context_menu->setItemEnabled(preview_id, false);
			}
		}
		else
		{
			bool val = item->childCount() != 0;
			context_menu->setItemEnabled(first_id, val);
			context_menu->setItemEnabled(normal_id, val);
			context_menu->setItemEnabled(last_id, val);
			context_menu->setItemEnabled(dnd_keep_id,val);
			context_menu->setItemEnabled(dnd_throw_away_id,val);
			
			
			if ( curr_tc->readyForPreview() && IsMultimediaFile(s.output_path))
			{
				context_menu->setItemEnabled(preview_id, true);
				preview_path = "cache";
			}
			else
				context_menu->setItemEnabled(preview_id, false);
		}

		context_menu->popup(p);
	}
	
	void FileView::contextItem(int id)
	{
		QPtrList<QListViewItem> sel = selectedItems();
		
		Priority newpriority = NORMAL_PRIORITY;
		if(id == this->preview_id)
		{
			new KRun(KURL::fromPathOrURL(this->curr_tc->getTorDir()+preview_path), 0, true, true);
			return;
		}
		else if (id == dnd_throw_away_id)
		{
			Uint32 n = sel.count();
			if (n == 1) // single item can be a directory
			{ 
				// the number of the beast > 1
				n = (*sel.begin())->childCount() == 0 ? 1 : 666;
			} 
			
			QString msg = i18n(
					"You will lose all data in this file, are you sure you want to do this ?",
					"You will lose all data in these files, are you sure you want to do this ?",n);
					
			if (KMessageBox::warningYesNo(0,msg) == KMessageBox::No)
				return; 
			newpriority = EXCLUDED;
		}
		else if(id == this->first_id)
		{
			newpriority = FIRST_PRIORITY;
		}
		else if(id == this->last_id)
		{
			newpriority = LAST_PRIORITY;
		}
		else if(id == this->normal_id)
		{
			newpriority = NORMAL_PRIORITY;
		}
		else if (id == dnd_keep_id)
		{
			newpriority = ONLY_SEED_PRIORITY;
		}
		

		
		QPtrList<QListViewItem>::Iterator i = sel.begin();
		while(i != sel.end())
		{
			QListViewItem* item = *i;
			changePriority(item, newpriority);
			multi_root->updatePriorityInformation(curr_tc);
			i++;
		}
	}
	
	void FileView::changePriority(QListViewItem* item, Priority newpriority)
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
		QListViewItem* myChild = item->firstChild();
		while( myChild )
		{
			changePriority(myChild, newpriority);
			myChild = myChild->nextSibling();
		}
	}
	
	void FileView::refreshFileTree(kt::TorrentInterface* tc)
	{
		if (!tc || curr_tc != tc)
			return;
		
		if (multi_root)
			multi_root->updateDNDInformation();
	}
}

#include "fileview.moc"
