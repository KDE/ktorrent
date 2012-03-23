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
#include <QApplication>
#include <QFont>
#include <kicon.h>
#include <kmimetype.h>
#include <util/log.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "downloadordermodel.h"

using namespace bt;

namespace kt
{

	DownloadOrderModel::DownloadOrderModel(bt::TorrentInterface* tor, QObject* parent) : QAbstractListModel(parent), tor(tor)
	{
		for(Uint32 i = 0; i < tor->getNumFiles(); i++)
		{
			order.append(i);
		}
	}


	DownloadOrderModel::~DownloadOrderModel()
	{
	}

	int DownloadOrderModel::rowCount(const QModelIndex & parent) const
	{
		if(!parent.isValid())
			return tor->getNumFiles();
		else
			return 0;
	}

	QVariant DownloadOrderModel::data(const QModelIndex & index, int role) const
	{
		if(!index.isValid())
			return QVariant();

		Uint32 idx = order.at(index.row());
		if(idx >= tor->getNumFiles())
			return QVariant();


		switch(role)
		{
		case Qt::DisplayRole:
			return tor->getTorrentFile(idx).getUserModifiedPath();
		case Qt::DecorationRole:
			return KIcon(KMimeType::findByPath(tor->getTorrentFile(idx).getPath())->iconName());
		case Qt::FontRole:
			if(!current_search_text.isEmpty() && tor->getTorrentFile(idx).getUserModifiedPath().contains(current_search_text, Qt::CaseInsensitive))
			{
				QFont font = QApplication::font();
				font.setBold(true);
				return font;
			}
		default:
			return QVariant();
		}
	}

	QModelIndex DownloadOrderModel::find(const QString& text)
	{
		current_search_text = text;
		for(Uint32 i = 0; i < tor->getNumFiles(); i++)
		{
			if(tor->getTorrentFile(i).getUserModifiedPath().contains(current_search_text, Qt::CaseInsensitive))
			{
				reset();
				return index(i);
			}
		}

		reset();
		return QModelIndex();
	}

	void DownloadOrderModel::clearHighLights()
	{
		current_search_text.clear();
		reset();
	}

	Qt::ItemFlags DownloadOrderModel::flags(const QModelIndex & index) const
	{
		Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

		if(index.isValid())
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
		QDataStream out(&data, QIODevice::WriteOnly);
		QList<Uint32> files;

		foreach(const QModelIndex & index, indexes)
		{
			if(index.isValid())
			{
				files.append(order.at(index.row()));
			}
		}
		out << files;
		mimeData->setData("application/octet-stream", data);
		return mimeData;
	}

	bool DownloadOrderModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
	{
		Q_UNUSED(column);
		if(action == Qt::IgnoreAction)
			return true;

		if(!data->hasFormat("application/octet-stream"))
			return false;

		int begin_row;
		if(row != -1)
			begin_row = row;
		else if(parent.isValid())
			begin_row = parent.row();
		else
			begin_row = rowCount(QModelIndex());

		QByteArray file_data = data->data("application/octet-stream");
		QDataStream in(&file_data, QIODevice::ReadOnly);
		QList<Uint32> files;
		in >> files;

		// remove all files from order which are in the dragged list
		int r = 0;
		for(QList<Uint32>::iterator i = order.begin(); i != order.end();)
		{
			if(files.contains(*i))
			{
				if(r < begin_row)    // if we remove something before the begin row, the row to insert decreases
					begin_row--;

				i = order.erase(i);
			}
			else
				i++;

			r++;
		}

		// reinsert dragged files
		foreach(Uint32 file, files)
		{
			order.insert(begin_row, file);
			begin_row++;
		}
		return true;
	}


	void DownloadOrderModel::moveUp(int row, int count)
	{
		if(row == 0)
			return;

		for(int i = 0; i < count; i++)
		{
			order.swap(row + i, row + i - 1);
		}

		emit dataChanged(createIndex(row - 1, 0), createIndex(row + count, 0));
	}

	void DownloadOrderModel::moveTop(int row, int count)
	{
		if(row == 0)
			return;

		QList<Uint32> tmp;
		for(int i = 0; i < count; i++)
		{
			tmp.append(order.takeAt(row));
		}

		order = tmp + order;
		reset();
	}

	void DownloadOrderModel::moveDown(int row, int count)
	{
		if(row + count >= (int)tor->getNumFiles())
			return;

		for(int i = count - 1; i >= 0; i--)
		{
			order.swap(row + i, row + i + 1);
		}

		emit dataChanged(createIndex(row, 0), createIndex(row + count + 1, 0));
	}

	void DownloadOrderModel::moveBottom(int row, int count)
	{
		if(row + count >= (int)tor->getNumFiles())
			return;

		QList<Uint32> tmp;
		for(int i = 0; i < count; i++)
		{
			tmp.append(order.takeAt(row));
		}

		order = order + tmp;
		reset();
	}

}
