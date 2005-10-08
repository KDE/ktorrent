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
#include <libtorrent/torrentcontrol.h>
#include "iwfiletreeitem.h"
#include "iwfiletreediritem.h"
#include "functions.h"


			
IWFileTreeItem::IWFileTreeItem(IWFileTreeDirItem* item,const QString & name,bt::TorrentFile & file)
	: QCheckListItem(item,QString::null,QCheckListItem::CheckBox),name(name),file(file)
{
	parent = item;
	manual_change = false;
	init();
}

IWFileTreeItem::~IWFileTreeItem()
{
}

void IWFileTreeItem::setChecked(bool on)
{
	manual_change = true;
	setOn(on);
	manual_change = false;
}

void IWFileTreeItem::init()
{
	setChecked(!file.doNotDownload());
	setText(0,name);
	setText(1,BytesToString(file.getSize()));
	setText(2,file.doNotDownload() ? i18n("No") : i18n("Yes"));
	setPixmap(0,KMimeType::findByPath(name)->pixmap(KIcon::Small));
}

void IWFileTreeItem::stateChange(bool on)
{
	file.setDoNotDownload(!on);
	setText(2,on ? i18n("Yes") : i18n("No"));
	if (!manual_change)
		parent->childStateChange();
}

void IWFileTreeItem::updatePreviewInformation(bt::TorrentControl* tc)
{
	if (file.isMultimedia())
	{
		if (tc->readyForPreview(file.getFirstChunk(), file.getFirstChunk()+1) )
		{
			setText(3, i18n("Available"));
		}
		else
		{
			setText(3, i18n("Pending"));
		}
	}
	else
		setText(3, i18n("No"));
}

int IWFileTreeItem::compare(QListViewItem* i, int col, bool ascending) const
{
	if (col == 1)
	{
		IWFileTreeItem* other = static_cast<IWFileTreeItem*>(i);
		return (int)(file.getSize() - other->file.getSize());
	}
	else
	{
		return QCheckListItem::compare(i, col, ascending);
	}
}
