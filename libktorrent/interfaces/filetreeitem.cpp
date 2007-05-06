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
#include <kmimetype.h>
#include <interfaces/functions.h>
#include <torrent/globals.h>
#include "filetreeitem.h"
#include "filetreediritem.h"
#include "torrentfileinterface.h"

using namespace bt;

namespace kt
{

	FileTreeItem::FileTreeItem(FileTreeDirItem* item,const QString & name,kt::TorrentFileInterface & file)
	: QCheckListItem(item,QString::null,QCheckListItem::CheckBox),name(name),file(file)
	{
		parent = item;
		manual_change = false;
		init();
	}

	FileTreeItem::~FileTreeItem()
	{
	}

	void FileTreeItem::setChecked(bool on,bool keep_data)
	{
		manual_change = true;
		setOn(on);
		manual_change = false;
		
		if (!on)
		{
			if (keep_data)
				file.setPriority(ONLY_SEED_PRIORITY);
			else
				file.setDoNotDownload(true);	
		}
		else
		{
			if (file.getPriority() == ONLY_SEED_PRIORITY)
				file.setPriority(NORMAL_PRIORITY);
			else
				file.setDoNotDownload(false);
		}
		
		updatePriorityText();
		parent->childStateChange();
	}
	
	void FileTreeItem::updatePriorityText()
	{
		switch(file.getPriority())
		{
			case FIRST_PRIORITY:
				setText(2,i18n("Yes, First"));
				break;
			case LAST_PRIORITY:
				setText(2,i18n("Yes, Last"));
				break;
			case EXCLUDED:
			case ONLY_SEED_PRIORITY:
				setText(2,i18n("No"));
				break;
			case PREVIEW_PRIORITY:
				break;
			default:
				setText(2,i18n("Yes"));
				break;
		}
	}

	void FileTreeItem::init()
	{
		manual_change = true;
		if (file.doNotDownload() || file.getPriority() == ONLY_SEED_PRIORITY)
			setOn(false);
		else
			setOn(true);
		manual_change = false;
		setText(0,name);
		setText(1,BytesToString(file.getSize()));
		updatePriorityText();
		setPixmap(0,KMimeType::findByPath(name)->pixmap(KIcon::Small));
	}

	void FileTreeItem::stateChange(bool on)
	{
		if (manual_change)
		{
			updatePriorityText();
			return;
		}
		
		if (!on)
		{
			switch (confirmationDialog())
			{
			case KEEP_DATA:
				file.setPriority(ONLY_SEED_PRIORITY);
				break;
			case THROW_AWAY_DATA:
				file.setDoNotDownload(true);
				break;
			case CANCELED:
			default:
				manual_change = true;
				setOn(true);
				manual_change = false;
				return;
			}	
		}
		else
		{
			if (file.getPriority() == ONLY_SEED_PRIORITY)
				file.setPriority(NORMAL_PRIORITY);
			else
				file.setDoNotDownload(false);
			
		}
		
		updatePriorityText();
		parent->childStateChange();
	}
	
	int FileTreeItem::compare(QListViewItem* i, int col, bool ascending) const
	{
		if (col == 1)
		{
			FileTreeItem* other = dynamic_cast<FileTreeItem*>(i);
			if (!other)
				return 0;
			else
				return (int)(file.getSize() - other->file.getSize());
		}
		else
		{
			// lets sort case insensitive
			return QString::compare(text(col).lower(),i->text(col).lower());
			//		QCheckListItem::compare(i, col, ascending);
		}
	}


	ConfirmationResult FileTreeItem::confirmationDialog()
	{
		if (file.isPreExistingFile())
			return KEEP_DATA;
		else
			return THROW_AWAY_DATA; 
	}
	
	Uint64 FileTreeItem::bytesToDownload() const
	{
		if (file.doNotDownload())
			return 0;
		else
			return file.getSize();
	}

}
