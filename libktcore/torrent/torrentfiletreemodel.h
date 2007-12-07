/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef KTTORRENTFILETREEMODEL_H
#define KTTORRENTFILETREEMODEL_H

#include <QAbstractItemModel>
#include <ktcore_export.h>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{

	/**
	 * Model for displaying file trees of a torrent
	 * @author Joris Guisson
	*/
	class KTCORE_EXPORT TorrentFileTreeModel : public QAbstractItemModel
	{
		Q_OBJECT
	public:
		enum DeselectMode
		{
			KEEP_FILES,DELETE_FILES
		};
		
		TorrentFileTreeModel(bt::TorrentInterface* tc,DeselectMode mode,QObject* parent);
		virtual ~TorrentFileTreeModel();
		
		virtual int rowCount(const QModelIndex & parent) const;
		virtual int columnCount(const QModelIndex & parent) const;
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		virtual QModelIndex parent(const QModelIndex & index) const;
		virtual QModelIndex index(int row,int column,const QModelIndex & parent) const;
		virtual Qt::ItemFlags flags(const QModelIndex & index) const;
		virtual bool setData(const QModelIndex & index, const QVariant & value, int role);
		
		/**
		 * Check all the files in the torrent.
		 */
		void checkAll();
		
		/**
		 * Uncheck all files in the torrent.
		 */
		void uncheckAll();
		
		/**
		 * Invert the check of each file of the torrent
		 */
		void invertCheck();
		
		/**
		 * Calculate the number of bytes to download
		 * @return Bytes to download
		 */
		bt::Uint64 bytesToDownload();
		
	signals:
		/**
		 * Emitted whenever one or more items changes check state
		 */
		void checkStateChanged();
		
	private: 
		void constructTree();
		void invertCheck(const QModelIndex & idx);

	private:
		struct Node;
		
		bt::TorrentInterface* tc;
		Node* root;
		DeselectMode mode;
		bool emit_check_state_change;
	};

}

#endif
