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
#include <QMimeData>
#include <QDataStream>
#include <kicon.h>
#include <kmimetype.h>
#include <util/log.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "downloadordermodel.h"

using namespace bt;

namespace kt
{

	DownloadOrderModel::DownloadOrderModel(bt::TorrentInterface* tor,QObject* parent) : QAbstractListModel(parent),tor(tor)
	{
		for (Uint32 i = 0;i < tor->getNumFiles();i++)
		{
			order.append(i);
		}
	}


	DownloadOrderModel::~DownloadOrderModel()
	{
	}
	
	int DownloadOrderModel::rowCount(const QModelIndex & parent) const
	{
		if (!parent.isValid())
			return tor->getNumFiles();
		else
			return 0;
	}
	
	QVariant DownloadOrderModel::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid())
			return QVariant();
		
		Uint32 idx = order.at(index.row());
		if (idx >= tor->getNumFiles())
			return QVariant();
		
		
		switch (role)
		{
			case Qt::DisplayRole:
				return tor->getTorrentFile(idx).getUserModifiedPath();
			case Qt::DecorationRole:
				return KIcon(KMimeType::findByPath(tor->getTorrentFile(idx).getPath())->iconName());
			default:
				return QVariant();
		}
	}
	
	Qt::ItemFlags DownloadOrderModel::flags(const QModelIndex & index) const
	{
		Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

		if (index.isValid())
			return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
		else
			return Qt::ItemIsDropEnabled | defaultFlags;
	}

	Qt::DropActions DownloadOrderModel::supportedDropActions() const
	{
		return Qt::CopyAction | Qt::MoveAction;
	}
	
	QStringList DownloadOrderModel::mimeTypes() const
	{
		QStringList types;
		types << "application/octet-stream";
		return types;
	}
	
	QMimeData* DownloadOrderModel::mimeData(const QModelIndexList &indexes) const
	{
		QMimeData *mimeData = new QMimeData();
		QByteArray data;
		QDataStream out(&data,QIODevice::WriteOnly);
		QList<Uint32> files;

		foreach (const QModelIndex &index, indexes) 
		{
			if (index.isValid()) 
			{
				files.append(order.at(index.row()));
			}
		}
		out << files;
		mimeData->setData("application/octet-stream",data);
		return mimeData;
	}
	
	bool DownloadOrderModel::dropMimeData(const QMimeData *data,Qt::DropAction action, int row, int column, const QModelIndex &parent)		
	{
		Q_UNUSED(column);
		if (action == Qt::IgnoreAction)
			return true;

		if (!data->hasFormat("application/octet-stream"))
			return false;
		
		int begin_row;
		if (row != -1)
			begin_row = row;
		else if (parent.isValid())
			begin_row = parent.row();
		else
			begin_row = rowCount(QModelIndex());
		
		QByteArray file_data = data->data("application/octet-stream");
		QDataStream in(&file_data,QIODevice::ReadOnly);
		QList<Uint32> files;
		in >> files;
		
		// remove all files from order which are in the dragged list
		int r = 0;
		for (QList<Uint32>::iterator i = order.begin();i != order.end();)
		{
			if (files.contains(*i))
			{
				if (r < begin_row) // if we remove something before the begin row, the row to insert decreases
					begin_row--; 
				
				i = order.erase(i);
			}
			else
				i++;
			
			r++;
		}
		
		// reinsert dragged files
		foreach (Uint32 file,files)
		{
			order.insert(begin_row,file);
			begin_row++;
		}
		return true;
	}
	
	
	void DownloadOrderModel::moveUp(const QModelIndex & index)
	{
		if (!index.isValid())
			return;
		
		int r = index.row();
		if (r == 0)
			return;
		
		order.swap(r,r-1);
		emit dataChanged(createIndex(r-1,0),index);
	}
	
	void DownloadOrderModel::moveDown(const QModelIndex & index)
	{
		if (!index.isValid())
			return;
		
		int r = index.row();
		if (r == tor->getNumFiles() - 1)
			return;
		
		order.swap(r,r+1);
		emit dataChanged(index,createIndex(r+1,0));
	}
}
