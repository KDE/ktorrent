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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <libutil/functions.h>
#include <libtorrent/torrentfile.h>
#include <libtorrent/torrentcontrol.h>
#include "iwfiletreediritem.h"
#include "iwfiletreeitem.h"
#include "functions.h"


IWFileTreeDirItem::IWFileTreeDirItem(KListView* klv,const QString & name)
	: QCheckListItem(klv,QString::null,QCheckListItem::CheckBox),name(name)
{
	parent = 0;
	size = 0;
	setPixmap(0,KGlobal::iconLoader()->loadIcon("folder",KIcon::Small));
	setText(0,name);
	setText(1,BytesToString(size));
	setText(2,i18n("Yes"));
	manual_change = true;
	setOn(true);
	manual_change = false;
}

IWFileTreeDirItem::IWFileTreeDirItem(IWFileTreeDirItem* parent,const QString & name)
	: QCheckListItem(parent,QString::null,QCheckListItem::CheckBox),
	name(name),parent(parent)
{
	size = 0;
	setPixmap(0,KGlobal::iconLoader()->loadIcon("folder",KIcon::Small));
	setText(0,name);
	setText(1,BytesToString(size));
	setText(2,i18n("Yes"));
	manual_change = true;
	setOn(true);
	manual_change = false;
}

IWFileTreeDirItem::~IWFileTreeDirItem()
{
}

void IWFileTreeDirItem::insert(const QString & path,bt::TorrentFile & file)
{
	size += file.getSize();
	setText(1,BytesToString(size));
	int p = path.find(bt::DirSeparator());
	if (p == -1)
	{
		children.insert(path,new IWFileTreeItem(this,path,file));
	}
	else
	{
		QString subdir = path.left(p);
		IWFileTreeDirItem* sd = subdirs.find(subdir);
		if (!sd)
		{
			sd = new IWFileTreeDirItem(this,subdir);
			subdirs.insert(subdir,sd);
		}
			
		sd->insert(path.mid(p+1),file);
	}
}

bt::TorrentFile & IWFileTreeDirItem::findTorrentFile(QListViewItem* item)
{
	// first see if item matches a child
	bt::PtrMap<QString,IWFileTreeItem>::iterator i = children.begin();
	while (i != children.end())
	{
		if (i->second == item)
			return i->second->getTorrentFile();
		i++;
	}

	// not found so go to subdirs and see if they can find it
	bt::PtrMap<QString,IWFileTreeDirItem>::iterator j = subdirs.begin();
	while (j != subdirs.end())
	{
		bt::TorrentFile & tf = j->second->findTorrentFile(item);
		if (!tf.isNull())
			return tf;
		j++;
	}

	// we haven't found anything
	// so return null
	return bt::TorrentFile::null;
}

void IWFileTreeDirItem::setAllChecked(bool on)
{
	if (!manual_change)
	{
		manual_change = true;
		setOn(on);
		manual_change = false;
	}
	// first set all the child items
	bt::PtrMap<QString,IWFileTreeItem>::iterator i = children.begin();
	while (i != children.end())
	{
		i->second->setChecked(on);
		i++;
	}

	// then recursivly move on to subdirs
	bt::PtrMap<QString,IWFileTreeDirItem>::iterator j = subdirs.begin();
	while (j != subdirs.end())
	{
		j->second->setAllChecked(on);
		j++;
	}
}

void IWFileTreeDirItem::invertChecked()
{
	// first set all the child items
	bt::PtrMap<QString,IWFileTreeItem>::iterator i = children.begin();
	while (i != children.end())
	{
		IWFileTreeItem* item = i->second;
		item->setChecked(!item->isOn());
		i++;
	}

	// then recursivly move on to subdirs
	bt::PtrMap<QString,IWFileTreeDirItem>::iterator j = subdirs.begin();
	while (j != subdirs.end())
	{
		j->second->invertChecked();
		j++;
	}
}

void IWFileTreeDirItem::updatePreviewInformation(bt::TorrentControl* tc)
{
	// first set all the child items
	bt::PtrMap<QString,IWFileTreeItem>::iterator i = children.begin();
	while (i != children.end())
	{
		IWFileTreeItem* item = i->second;
		item->updatePreviewInformation(tc);
		i++;
	}

	// then recursivly move on to subdirs
	bt::PtrMap<QString,IWFileTreeDirItem>::iterator j = subdirs.begin();
	while (j != subdirs.end())
	{
		j->second->updatePreviewInformation(tc);
		j++;
	}
}

void IWFileTreeDirItem::stateChange(bool on)
{
	if (!manual_change)
	{
		setAllChecked(on);
	}
	setText(2,on ? i18n("Yes") : i18n("No"));
}

bool IWFileTreeDirItem::allChildrenOn()
{
	// first check all the child items
	bt::PtrMap<QString,IWFileTreeItem>::iterator i = children.begin();
	while (i != children.end())
	{
		IWFileTreeItem* item = i->second;
		if (!item->isOn())
			return false;
		i++;
	}

	// then recursivly move on to subdirs
	bt::PtrMap<QString,IWFileTreeDirItem>::iterator j = subdirs.begin();
	while (j != subdirs.end())
	{
		if (!j->second->allChildrenOn())
			return false;
		j++;
	}
	return true;
}

void IWFileTreeDirItem::childStateChange()
{
	// only set this dir on if all children are on
	manual_change = true;
	setOn(allChildrenOn());
	manual_change = false;
	
	if (parent)
		parent->childStateChange();
}

