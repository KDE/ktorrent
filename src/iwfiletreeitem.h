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
#ifndef IWFILETREEITEM_H
#define IWFILETREEITEM_H

#include <klistview.h>
#include <libutil/constants.h>
#include <libutil/ptrmap.h>

class IWFileTreeDirItem;

using bt::Uint32;

namespace bt
{
	class TorrentFile;
}

/**
@author Joris Guisson
*/
class IWFileTreeItem : public QCheckListItem
{
	QString name;
	Uint32 size;
	bt::TorrentFile & file;
public:
	IWFileTreeItem(KListView* lv,const QString & name,bt::TorrentFile & file);
	IWFileTreeItem(IWFileTreeDirItem* item,const QString & name,bt::TorrentFile & file);
	virtual ~IWFileTreeItem();

private:
	void init();
	virtual void stateChange(bool on);
};

class IWFileTreeDirItem : public KListViewItem
{
	QString name;
	bt::PtrMap<QString,IWFileTreeItem> children;
	bt::PtrMap<QString,IWFileTreeDirItem> subdirs;
public:
	IWFileTreeDirItem(KListView* klv,const QString & name);
	IWFileTreeDirItem(IWFileTreeDirItem* parent,const QString & name);
	virtual ~IWFileTreeDirItem();

	void insert(const QString & path,bt::TorrentFile & file);
};

#endif
