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

	void FileTreeItem::setChecked(bool on)
	{
		manual_change = true;
		setOn(on);
		manual_change = false;
	}

	void FileTreeItem::init()
	{
		setChecked(!file.doNotDownload());
		setText(0,name);
		setText(1,BytesToString(file.getSize()));
		switch(file.getPriority())
		{
		case FIRST_PRIORITY:
			setText(2,i18n("Yes, First"));
			break;
		case LAST_PRIORITY:
			setText(2,i18n("Yes, Last"));
			break;
		case EXCLUDED:
			setText(2,i18n("No"));
			break;
		case PREVIEW_PRIORITY:
			break;
		default:
			setText(2,i18n("Yes"));
			break;
		}
		setPixmap(0,KMimeType::findByPath(name)->pixmap(KIcon::Small));
	}

	void FileTreeItem::stateChange(bool on)
	{
		if (!manual_change && !on)
		{
			if (!deselectOK())
			{
				manual_change = true;
				setOn(true);
				manual_change = false;
				return;
			}	
		}
		Globals::instance().setCriticalOperationMode(true);
		file.setDoNotDownload(!on);
		Globals::instance().setCriticalOperationMode(false);
		switch(file.getPriority())
		{
		case FIRST_PRIORITY:
			setText(2,i18n("Yes, First"));
			break;
		case LAST_PRIORITY:
			setText(2,i18n("Yes, Last"));
			break;
		case EXCLUDED:
			setText(2,i18n("No"));
			break;
		case PREVIEW_PRIORITY:
			break;
		default:
			setText(2,i18n("Yes"));
			break;
		}
		if (!manual_change)
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
			return QCheckListItem::compare(i, col, ascending);
		}
	}


	bool FileTreeItem::deselectOK()
	{
		return true;
	}

}
