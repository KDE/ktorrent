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
#ifndef IWFILETREEITEM_H
#define IWFILETREEITEM_H

#include <qobject.h>
#include <interfaces/filetreeitem.h>

using namespace bt;

namespace kt
{
	class TorrentFileInterface;
	class TorrentInterface;
	class IWFileTreeDirItem;

	/**
	* @author Joris Guisson
	*
	* File item in the InfoWidget's file view.
	*/
	class IWFileTreeItem : public QObject, public kt::FileTreeItem
	{
		Q_OBJECT
				
		double perc_complete;
	public:
		IWFileTreeItem(IWFileTreeDirItem* item,const QString & name,kt::TorrentFileInterface & file);
		virtual ~IWFileTreeItem();
	
		void updatePreviewInformation(kt::TorrentInterface* tc);
		void updatePercentageInformation();
		void updatePriorityInformation(kt::TorrentInterface* tc);
		void updateDNDInformation();
	protected:
		virtual int compare(QListViewItem* i, int col, bool ascending) const;
		virtual bt::ConfirmationResult confirmationDialog();
		
	protected slots:
		void onPercentageUpdated(float p);
		void onPreviewAvailable(bool av);
	};
}



#endif
