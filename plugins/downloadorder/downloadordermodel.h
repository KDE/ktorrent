/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#ifndef KTDOWNLOADORDERMODEL_H
#define KTDOWNLOADORDERMODEL_H

#include <QAbstractListModel>
#include <util/constants.h>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{

	/**
		Model for the download order in the dialog
	*/
	class DownloadOrderModel : public QAbstractListModel
	{
		Q_OBJECT
	public:
		DownloadOrderModel(bt::TorrentInterface* tor,QObject* parent);
		virtual ~DownloadOrderModel();
		
		/// Initialize the order
		void initOrder(const QList<bt::Uint32> & sl) {order = sl;}
		
		/// Get the order
		const QList<bt::Uint32> & downloadOrder() const {return order;}

		virtual int rowCount(const QModelIndex & parent) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		virtual Qt::ItemFlags flags(const QModelIndex & index) const;
		virtual Qt::DropActions supportedDropActions() const;
		virtual QStringList mimeTypes() const;
		virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
		virtual bool dropMimeData(const QMimeData *data,Qt::DropAction action, int row, int column, const QModelIndex &parent);
	
		void moveUp(const QModelIndex & index);
		void moveDown(const QModelIndex & index);
	
	private:
		bt::TorrentInterface* tor;
		QList<bt::Uint32> order;
	};

}

#endif
