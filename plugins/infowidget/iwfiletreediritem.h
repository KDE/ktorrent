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
#ifndef IWFILETREEDIRITEM_H
#define IWFILETREEDIRITEM_H

#include <interfaces/filetreediritem.h>

class IWFileTreeItem;

using bt::Uint32;
using bt::Priority;
using bt::FIRST_PRIORITY;
using bt::NORMAL_PRIORITY;
using bt::LAST_PRIORITY;
using bt::PREVIEW_PRIORITY;
using bt::EXCLUDED;

namespace bt
{
	class TorrentFile;
	class TorrentInterface;
}

namespace kt
{
	using namespace bt;
	/**
	* @author Joris Guisson
	*
	* Directory item in the InfoWidget's file view.
	*/
	class IWFileTreeDirItem : public kt::FileTreeDirItem
	{
	public:
		IWFileTreeDirItem(KListView* klv,const QString & name);
		IWFileTreeDirItem(IWFileTreeDirItem* parent,const QString & name);
		virtual ~IWFileTreeDirItem();
	
		/**
		* Update the preview information.
		* @param tc The TorrentInterface object
		*/
		void updatePreviewInformation(kt::TorrentInterface* tc);

		/**
		 * Update the downloaded percentage information.
		 */
		void updatePercentageInformation();

		Priority updatePriorityInformation(kt::TorrentInterface* tc);
		
		/**
		 * Update the DND information of each file item.
		*/
		void updateDNDInformation();

		virtual kt::FileTreeItem* newFileTreeItem(const QString & name, kt::TorrentFileInterface & file);
		virtual kt::FileTreeDirItem* newFileTreeDirItem(const QString & subdir);
		virtual bt::ConfirmationResult confirmationDialog();
	};
}

#endif
