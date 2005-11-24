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
		
	
	void IWFileTreeDirItem::updatePercentageInformation(kt::TorrentInterface* tc)
	{
		// first set all the child items
		bt::PtrMap<QString,FileTreeItem>::iterator i = children.begin();
		while (i != children.end())
		{
			IWFileTreeItem* item = (IWFileTreeItem*)i->second;
			item->updatePercentageInformation(tc);
			i++;
		}
	
		// then recursivly move on to subdirs
		bt::PtrMap<QString,FileTreeDirItem>::iterator j = subdirs.begin();
		while (j != subdirs.end())
		{
			((IWFileTreeDirItem*)j->second)->updatePercentageInformation(tc);
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
	
	FileTreeItem* IWFileTreeDirItem::newFileTreeItem(const QString & name,TorrentFileInterface & file)
	{
		return new IWFileTreeItem(this,name,file);
	}
	
	FileTreeDirItem* IWFileTreeDirItem::newFileTreeDirItem(const QString & subdir)
	{
		return new IWFileTreeDirItem(this,subdir);
	}
}