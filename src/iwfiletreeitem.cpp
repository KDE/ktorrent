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
#include <kmimetype.h>
#include <libutil/functions.h>
#include <libtorrent/torrentfile.h>
#include "iwfiletreeitem.h"
#include "functions.h"


IWFileTreeItem::IWFileTreeItem(KListView* lv,const QString & name,bt::TorrentFile & file)
	: QCheckListItem(lv,QString::null,QCheckListItem::CheckBox),name(name),file(file)
{
	init();
}
			
IWFileTreeItem::IWFileTreeItem(IWFileTreeDirItem* item,const QString & name,bt::TorrentFile & file)
	: QCheckListItem(item,QString::null,QCheckListItem::CheckBox),name(name),file(file)
{
	init();
}

IWFileTreeItem::~IWFileTreeItem()
{
}

void IWFileTreeItem::init()
{
	setOn(!file.doNotDownload());
	setText(0,name);
	setText(1,BytesToString(file.getSize()));
	setText(2,file.doNotDownload() ? i18n("No") : i18n("Yes"));
	setPixmap(0,KMimeType::findByPath(name)->pixmap(KIcon::Small));
}

void IWFileTreeItem::stateChange(bool on)
{
	file.setDoNotDownload(!on);
	setText(2,on ? i18n("Yes") : i18n("No"));
}
////////////////////////////////////////////////////

IWFileTreeDirItem::IWFileTreeDirItem(KListView* klv,const QString & name)
	: KListViewItem(klv),name(name)
{
	setPixmap(0,KGlobal::iconLoader()->loadIcon("folder",KIcon::Small));
	setText(0,name);
}

IWFileTreeDirItem::IWFileTreeDirItem(IWFileTreeDirItem* parent,const QString & name)
	: KListViewItem(parent),name(name)
{
	setPixmap(0,KGlobal::iconLoader()->loadIcon("folder",KIcon::Small));
	setText(0,name);
}

IWFileTreeDirItem::~IWFileTreeDirItem()
{
}

void IWFileTreeDirItem::insert(const QString & path,bt::TorrentFile & file)
{
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