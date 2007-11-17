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
#include <interfaces/functions.h>
#include <torrent/globals.h>
#include "filetreediritem.h"
#include "filetreeitem.h"
#include <interfaces/torrentfileinterface.h>
#include <torrent/torrentfile.h>

using namespace bt;

namespace kt
{

	FileTreeDirItem::FileTreeDirItem(QTreeWidget* klv,const QString & name,FileTreeRootListener* rl) : QTreeWidgetItem(klv),name(name),root_listener(rl)
	{
		parent = 0;
		size = 0;
		setIcon(0,SmallIcon("folder"));
		setText(0,name);
		setText(1,BytesToString(size));
		setText(2,i18n("Yes"));
		manual_change = true;
		setCheckState(0,Qt::Checked);
		manual_change = false;
	}

	FileTreeDirItem::FileTreeDirItem(FileTreeDirItem* parent,const QString & name)
	: QTreeWidgetItem(parent),
	name(name),parent(parent)
	{
		size = 0;
		setIcon(0,SmallIcon("folder"));
		setText(0,name);
		setText(1,BytesToString(size));
		setText(2,i18n("Yes"));
		manual_change = true;
		setCheckState(0,Qt::Checked);
		manual_change = false;
	}

	FileTreeDirItem::~FileTreeDirItem()
	{
	}

	void FileTreeDirItem::insert(const QString & path,bt::TorrentFileInterface & file,DeselectOptions options)
	{
		size += file.getSize();
		setText(1,BytesToString(size));
		int p = path.indexOf(bt::DirSeparator());
		if (p == -1)
		{
			children.insert(path,newFileTreeItem(path,file,options));
		}
		else
		{
			QString subdir = path.left(p);
			FileTreeDirItem* sd = subdirs.find(subdir);
			if (!sd)
			{
				sd = newFileTreeDirItem(subdir);
				subdirs.insert(subdir,sd);
			}
			
			sd->insert(path.mid(p+1),file,options);
		}
	}
	
	void FileTreeDirItem::setAllChecked(bool on,bool keep_data)
	{
		if (!manual_change)
		{
			manual_change = true;
			setCheckState(0,on ? Qt::Checked : Qt::Unchecked);
			manual_change = false;
		}
		// first set all the child items
		bt::PtrMap<QString,FileTreeItem>::iterator i = children.begin();
		while (i != children.end())
		{
			i->second->setChecked(on,keep_data);
			i++;
		}

		// then recursivly move on to subdirs
		bt::PtrMap<QString,FileTreeDirItem>::iterator j = subdirs.begin();
		while (j != subdirs.end())
		{
			j->second->setAllChecked(on,keep_data);
			j++;
		}
	}
	

	void FileTreeDirItem::invertChecked()
	{
		// first set all the child items
		bt::PtrMap<QString,FileTreeItem>::iterator i = children.begin();
		while (i != children.end())
		{
			FileTreeItem* item = i->second;
			item->setChecked(!item->isOn());
			i++;
		}

		// then recursivly move on to subdirs
		bt::PtrMap<QString,FileTreeDirItem>::iterator j = subdirs.begin();
		while (j != subdirs.end())
		{
			j->second->invertChecked();
			j++;
		}
	}

	void FileTreeDirItem::stateChange(bool on)
	{
		if (!manual_change)
		{
			if (on)
			{
				setAllChecked(true);
			}
			else
			{
				setAllChecked(false,true);
			}
			if (parent)
				parent->childStateChange();
		}
		setText(2,on ? i18n("Yes") : i18n("No"));
	}
	
	Uint64 FileTreeDirItem::bytesToDownload() const
 	{
 		Uint64 tot = 0;
 		// first check all the child items
 		bt::PtrMap<QString,FileTreeItem>::const_iterator i = children.begin();
 		while (i != children.end())
 		{
 			const FileTreeItem* item = i->second;
 			tot += item->bytesToDownload();
 			i++;
 		}
  
 		// then recursivly move on to subdirs
 		bt::PtrMap<QString,FileTreeDirItem>::const_iterator j = subdirs.begin();
 		while (j != subdirs.end())
 		{
 			tot += j->second->bytesToDownload();
 			j++;
 		}
 		return tot;
 	}

	bool FileTreeDirItem::allChildrenOn()
	{
		// first check all the child items
		bt::PtrMap<QString,FileTreeItem>::iterator i = children.begin();
		while (i != children.end())
		{
			FileTreeItem* item = i->second;
			if (!item->isOn())
				return false;
			i++;
		}

		// then recursivly move on to subdirs
		bt::PtrMap<QString,FileTreeDirItem>::iterator j = subdirs.begin();
		while (j != subdirs.end())
		{
			if (!j->second->allChildrenOn())
				return false;
			j++;
		}
		return true;
	}

	void FileTreeDirItem::childStateChange()
	{
		// only set this dir on if all children are on
		manual_change = true;
		setCheckState(0,allChildrenOn() ? Qt::Checked : Qt::Unchecked);
		manual_change = false;
	
		if (parent)
			parent->childStateChange();
		else if (root_listener)
			root_listener->treeItemChanged();
			
	}
#if 0
	int FileTreeDirItem::compare(QListViewItem* i, int col, bool ascending) const
	{
		if (col == 1)
		{
			FileTreeDirItem* other = dynamic_cast<FileTreeDirItem*>(i);
			if (!other)
				return 0;
			else
				return (int)(size - other->size);
		}
		else
		{
			//return QCheckListItem::compare(i, col, ascending);
			// case insensitive comparison
			return QString::compare(text(col).toLower(),i->text(col).toLower());
		}
	}
#endif

	TorrentFileInterface & FileTreeDirItem::findTorrentFile(QTreeWidgetItem* item)
	{
		// first check all the child items
		TorrentFileInterface & nullfile = (TorrentFileInterface &)TorrentFile::null;
		bt::PtrMap<QString,FileTreeItem>::iterator i = children.begin();
		while (i != children.end())
		{
			FileTreeItem* file = i->second;
			if (file == (FileTreeItem*)item)
				return file->getTorrentFile();
			i++;
		}

		// then recursivly move on to subdirs
		bt::PtrMap<QString,FileTreeDirItem>::iterator j = subdirs.begin();
		while (j != subdirs.end())
		{
			TorrentFileInterface & thefile = j->second->findTorrentFile(item);
			if(!thefile.isNull())
				return thefile;
			j++;
		}
		return nullfile;
	}

	FileTreeItem* FileTreeDirItem::newFileTreeItem(const QString & name,TorrentFileInterface & file,DeselectOptions options)
	{
		return new FileTreeItem(this,name,file,options);
	}

	FileTreeDirItem* FileTreeDirItem::newFileTreeDirItem(const QString & subdir)
	{
		return new FileTreeDirItem(this,subdir);
	}

	QString FileTreeDirItem::getPath() const
	{
		if (!parent)
			return bt::DirSeparator();
		else
			return parent->getPath() + name + bt::DirSeparator();
	}

	void FileTreeDirItem::setData(int column, int role, const QVariant& value)
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

