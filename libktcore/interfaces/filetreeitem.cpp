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
#include <util/functions.h>
#include <torrent/globals.h>
#include <interfaces/torrentfileinterface.h>
#include "filetreeitem.h"
#include "filetreediritem.h"


using namespace bt;

namespace kt
{

	FileTreeItem::FileTreeItem(FileTreeDirItem* item,const QString & name,
				   bt::TorrentFileInterface & file,DeselectOptions options)
	: QTreeWidgetItem(item),name(name),file(file),options(options)
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
		setCheckState(0,on ? Qt::Checked : Qt::Unchecked);
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
		setIcon(0,SmallIcon(KMimeType::findByPath(name)->iconName()));
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
			if (options == KEEP_FILES)
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
#if 0	
	int FileTreeItem::compare(QTreeWidgetItem* i, int col, bool ascending) const
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
			return QString::compare(text(col).toLower(),i->text(col).toLower());
			//		QCheckListItem::compare(i, col, ascending);
		}
	}
#endif

	Uint64 FileTreeItem::bytesToDownload() const
	{
		if (file.doNotDownload())
			return 0;
		else
			return file.getSize();
	}

	void FileTreeItem::setData(int column, int role, const QVariant& value)
	{
		if (role == Qt::CheckStateRole)
		{
			Qt::CheckState newState = static_cast<Qt::CheckState>(value.toInt());
			Qt::CheckState oldState = static_cast<Qt::CheckState>(data(column, role).toInt());
					
			QTreeWidgetItem::setData(column, role, value);
							
			if (newState != oldState)
			{
				stateChange(newState == Qt::Checked);
			}
		}
		else
		{
			QTreeWidgetItem::setData(column, role, value);
		}
	}
}
