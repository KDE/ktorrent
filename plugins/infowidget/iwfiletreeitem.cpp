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
#include <klocale.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "iwfiletreeitem.h"
#include "iwfiletreediritem.h"
#include "functions.h"

using namespace kt;

namespace kt
{
				
	IWFileTreeItem::IWFileTreeItem(IWFileTreeDirItem* item,const QString & name,kt::TorrentFileInterface & file)
		: FileTreeItem(item,name,file)
	{
	}
	
	IWFileTreeItem::~IWFileTreeItem()
	{
	}
	
	void IWFileTreeItem::updatePreviewInformation(kt::TorrentInterface* tc)
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

}