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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <util/functions.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include "iwfiletreediritem.h"
#include "iwfiletreeitem.h"
#include "functions.h"

using namespace kt;

namespace kt
{
	
	IWFileTreeDirItem::IWFileTreeDirItem(KListView* klv,const QString & name)
		: kt::FileTreeDirItem(klv,name)
	{
	}
	
	IWFileTreeDirItem::IWFileTreeDirItem(IWFileTreeDirItem* parent,const QString & name)
		: kt::FileTreeDirItem(parent,name)
	{
	}
	
	IWFileTreeDirItem::~IWFileTreeDirItem()
	{
	}
		
	
	void IWFileTreeDirItem::updatePercentageInformation()
	{
		// first set all the child items
		bt::PtrMap<QString,FileTreeItem>::iterator i = children.begin();
		while (i != children.end())
		{
			IWFileTreeItem* item = (IWFileTreeItem*)i->second;
			item->updatePercentageInformation();
			i++;
		}
	
		// then recursivly move on to subdirs
		bt::PtrMap<QString,FileTreeDirItem>::iterator j = subdirs.begin();
		while (j != subdirs.end())
		{
			((IWFileTreeDirItem*)j->second)->updatePercentageInformation();
			j++;
		}
	}
	
	void IWFileTreeDirItem::updatePreviewInformation(kt::TorrentInterface* tc)
	{
		// first set all the child items
		bt::PtrMap<QString,FileTreeItem>::iterator i = children.begin();
		while (i != children.end())
		{
			IWFileTreeItem* item = (IWFileTreeItem*)i->second;
			item->updatePreviewInformation(tc);
			i++;
		}
	
		// then recursivly move on to subdirs
		bt::PtrMap<QString,FileTreeDirItem>::iterator j = subdirs.begin();
		while (j != subdirs.end())
		{
			((IWFileTreeDirItem*)j->second)->updatePreviewInformation(tc);
			j++;
		}
	}
	
	Priority IWFileTreeDirItem::updatePriorityInformation(kt::TorrentInterface* tc)
	{
                // first set all the child items
		bt::PtrMap<QString,FileTreeItem>::iterator i = children.begin();
		bool setpriority = false;
		bool oneexcluded = false;
		Priority priority = PREVIEW_PRIORITY;
		if(i != children.end())
		{
			IWFileTreeItem* item = (IWFileTreeItem*)i->second;
			item->updatePriorityInformation(tc);
			i++;
			priority = item->getTorrentFile().getPriority();
			if(priority == EXCLUDED)
				oneexcluded = true;
			setpriority = true;
		}
		while (i != children.end())
		{
			IWFileTreeItem* item = (IWFileTreeItem*)i->second;
			item->updatePriorityInformation(tc);
			i++;
			if(item->getTorrentFile().getPriority() != priority)
				setpriority = false;
			if(item->getTorrentFile().getPriority() == EXCLUDED)
				oneexcluded = true;
		}

                // then recursivly move on to subdirs
		bt::PtrMap<QString,FileTreeDirItem>::iterator j = subdirs.begin();
		if(j != subdirs.end() && children.begin() == children.end())
		{
			Priority priority =
					((IWFileTreeDirItem*)j->second)->updatePriorityInformation(tc);
			if(priority != PREVIEW_PRIORITY)
				setpriority = true;
			if(priority == EXCLUDED)
				oneexcluded = true;
			j++;
		}

		while (j != subdirs.end())
		{
			if(((IWFileTreeDirItem*)j->second)->updatePriorityInformation(tc)
						  != priority)
				setpriority = false;
			if(((IWFileTreeDirItem*)j->second)->updatePriorityInformation(tc)
						  == EXCLUDED)
				oneexcluded = true;
			j++;
		}

		if(setpriority)
		{
			switch(priority)
			{
				case FIRST_PRIORITY:
					setText(2, i18n("Yes, First"));
					childStateChange();
					break;
				case LAST_PRIORITY:
					setText(2, i18n("Yes, Last"));
					childStateChange();
					break;
				case EXCLUDED:
					setText(2, i18n("No"));
					childStateChange();
					break;
				default:
					setText(2, i18n("Yes"));
					childStateChange();
					break;
			}
			return priority;
		}
		if(oneexcluded)
		{
			setText(2, i18n("No"));
			childStateChange();
		}
		else
		{
			setText(2, i18n("Yes"));
			childStateChange();
		}
		return PREVIEW_PRIORITY;
	}

	FileTreeItem* IWFileTreeDirItem::newFileTreeItem(const QString & name,TorrentFileInterface & file)
	{
		return new IWFileTreeItem(this,name,file);
	}
	
	FileTreeDirItem* IWFileTreeDirItem::newFileTreeDirItem(const QString & subdir)
	{
		return new IWFileTreeDirItem(this,subdir);
	}
	
	void IWFileTreeDirItem::updateDNDInformation()
	{
			// first set all the child items
		bt::PtrMap<QString,FileTreeItem>::iterator i = children.begin();
		while (i != children.end())
		{
			IWFileTreeItem* item = (IWFileTreeItem*)i->second;
			item->updateDNDInformation();
			i++;
		}
	
		// then recursivly move on to subdirs
		bt::PtrMap<QString,FileTreeDirItem>::iterator j = subdirs.begin();
		while (j != subdirs.end())
		{
			((IWFileTreeDirItem*)j->second)->updateDNDInformation();
			j++;
		}
	}

	bt::ConfirmationResult IWFileTreeDirItem::confirmationDialog()
	{
		return bt::KEEP_DATA;
/*		QString msg = i18n("Do you want to keep the existing data for seeding ?");
		int ret = KMessageBox::warningYesNoCancel(0,msg,QString::null,
				KGuiItem(i18n("Keep the data")),
				KGuiItem(i18n("Delete the data")));
		if (ret == KMessageBox::Yes)
			return bt::KEEP_DATA;
		else if (ret == KMessageBox::No)
			return bt::THROW_AWAY_DATA;
		else
			return bt::CANCELED;
		*/
	}
}
