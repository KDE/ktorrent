/***************************************************************************
*   Copyright (C) 2009 by Joris Guisson                                   *
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
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
***************************************************************************/

#include "playlist.h"
#include <QTime>
#include <QFile>
#include <QUrl>
#include <QMimeData>
#include <QStringList>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <klocale.h>
#include <QFileInfo>
#include <util/log.h>
#include <qtextstream.h>

using namespace bt;

namespace kt
{
	PlayList::PlayList(QObject* parent) : QAbstractItemModel(parent)
	{
	}
	
	PlayList::~PlayList() 
	{
	}
	
	void PlayList::addFile(const QString & file)
	{
		files.append(file);
		insertRow(files.count() - 1);
	}
	
	void PlayList::removeFile(const QString & file)
	{
		int i = files.indexOf(file);
		if (i >= 0)
			removeRow(i);
	}
	
	QString PlayList::fileForIndex(const QModelIndex& index) const
	{
		if (!index.isValid())
			return QString();
		else
			return files.at(index.row());
	}
	
	void PlayList::clear() 
	{
		files.clear();
		reset();
	}

	
	QModelIndex PlayList::next(const QModelIndex & idx,bool random) const
	{
		if (files.count() == 0)
			return QModelIndex();
		
		if (!idx.isValid())
		{
			if (!random)
			{
				return index(0,0,QModelIndex());
			}
			else
			{
				return randomNext(QModelIndex());
			}
		}
		else if (!random)
		{
			return next(idx);
		}
		else
		{
			return randomNext(idx);
		}
	}
	
	QModelIndex PlayList::next(const QModelIndex & idx) const
	{
		return idx.sibling(idx.row()+1,0); // take a look at the next sibling
	}
	
	QModelIndex PlayList::randomNext(const QModelIndex & idx) const
	{
		if (files.count() <= 1)
			return QModelIndex();
		
		int r = qrand() % files.count();
		while (r == idx.row())
			r = qrand() % files.count();
		
		return index(r,0,QModelIndex());
	}
	
	QVariant PlayList::headerData(int section, Qt::Orientation orientation, int role) const 
	{
		if (orientation == Qt::Vertical || role != Qt::DisplayRole)
			return QVariant();
		
		switch (section)
		{
			case 0: return i18n("Title");
			case 1: return i18n("Artist");
			case 2: return i18n("Album");
			case 3: return i18n("Length");
			case 4: return i18n("Year");
			default: return QVariant();
		}
	}

	QVariant PlayList::data(const QModelIndex& index, int role) const
	{
		if (!index.isValid() || role != Qt::DisplayRole)
			return QVariant();
		
		QString file = files.at(index.row());
		TagLib::FileRef ref(QFile::encodeName(file).data(),true,TagLib::AudioProperties::Fast);
		if (ref.isNull())
		{
			if (index.column() == 0)
				return QFileInfo(file).fileName();
			else
				return QVariant();
		}
		
		TagLib::Tag* tag = ref.tag();
		if (!tag)
		{
			if (index.column() == 0)
				return QFileInfo(file).fileName();
			else
				return QVariant();
		}
		
		switch (index.column())
		{
			case 0: return QString(tag->title().toCString(true));
			case 1: return QString(tag->artist().toCString(true));
			case 2: return QString(tag->album().toCString(true));
			case 3: 
			{
				QTime t(0,0);
				t = t.addSecs(ref.audioProperties()->length());
				return t.toString("m:ss");
			}
			case 4: return tag->year() == 0 ? QVariant() : tag->year();
			default: return QVariant();
		}
	}

	int PlayList::columnCount(const QModelIndex& parent) const 
	{
		if (parent.isValid())
			return 0;
		else
			return 5;
	}

	int PlayList::rowCount(const QModelIndex& parent) const 
	{
		return parent.isValid() ? 0 : files.count();
	}

	QModelIndex PlayList::parent(const QModelIndex& child) const 
	{
		Q_UNUSED(child);
		return QModelIndex();
	}

	QModelIndex PlayList::index(int row, int column, const QModelIndex& parent) const 
	{
		if (parent.isValid())
			return QModelIndex();
		else
			return createIndex(row,column);
	}
	
	Qt::DropActions PlayList::supportedDropActions() const
	{
		return Qt::CopyAction | Qt::MoveAction;
	}

	Qt::ItemFlags PlayList::flags(const QModelIndex & index) const
	{
		Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
		
		if (index.isValid())
			return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
		else
			return Qt::ItemIsDropEnabled | defaultFlags;
	}
	
	QStringList PlayList::mimeTypes() const
	{
		QStringList types;
		types << "text/uri-list";
		return types;
	}
	
	QMimeData* PlayList::mimeData(const QModelIndexList &indexes) const
	{
		dragged_rows.clear();
		QMimeData* data = new QMimeData();
		QList<QUrl> urls;
		foreach (QModelIndex index, indexes)
		{
			if (index.isValid() && index.column() == 0) 
			{
				QString text = files.at(index.row());
				urls.append(text);
				dragged_rows.append(index.row());
			}
		}
		
		data->setUrls(urls);
		return data;
	}
	
	bool PlayList::dropMimeData(const QMimeData *data,Qt::DropAction action, int row, int column, const QModelIndex &parent)
	{
		if (action == Qt::IgnoreAction)
			return true;
		
		QList<QUrl> urls = data->urls();
		if (urls.count() == 0 || column > 0)
			return false;
		
		if (row == -1)
			row = parent.row();
		
		if (row == -1)
			row = rowCount(QModelIndex());
		
		// Remove dragged rows if there are any
		qSort(dragged_rows);
		int nr = 0;
		foreach (int r,dragged_rows)
		{
			r -= nr;
			removeRow(r);
			nr++;
		}
		
		row -= nr;
		
		foreach (const QUrl & url,urls)
		{
			files.insert(row,url.path());
		}
		insertRows(row,urls.count(),QModelIndex());
		dragged_rows.clear();
		return true;
	}
	
	bool PlayList::insertRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	bool PlayList::removeRows(int row,int count,const QModelIndex & parent) 
	{
		Q_UNUSED(parent);
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		for (int i = 0;i < count;i++)
			files.removeAt(i + row);
		endRemoveRows();
		return true;
	}
	
	void PlayList::save(const QString & file)
	{
		QFile fptr(file);
		if (!fptr.open(QIODevice::WriteOnly))
		{
			Out(SYS_GEN|LOG_NOTICE) << "Failed to open file " << file << endl;
			return;
		}
		
		QTextStream out(&fptr);
		foreach (const QString & f,files)
			out << f << endl;
	}
	
	void PlayList::load(const QString & file)
	{
		QFile fptr(file);
		if (!fptr.open(QIODevice::ReadOnly))
		{
			Out(SYS_GEN|LOG_NOTICE) << "Failed to open file " << file << endl;
			return;
		}
		
		QTextStream in(&fptr);
		while (!in.atEnd())
		{
			files.append(in.readLine());
		}
	}
}