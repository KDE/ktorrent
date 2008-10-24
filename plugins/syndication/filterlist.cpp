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
#include <QFile>
#include <kicon.h>
#include <util/log.h>
#include <util/error.h>
#include <bcodec/bnode.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include "filterlist.h"
#include "filter.h"

using namespace bt;

namespace kt
{

	FilterList::FilterList(QObject* parent)
			: QAbstractListModel(parent)
	{
	}


	FilterList::~FilterList()
	{
		qDeleteAll(filters);
	}
	
	void FilterList::addFilter(Filter* f)
	{
		filters.append(f);
		insertRow(filters.count() - 1);
	}
	
	void FilterList::removeFilter(Filter* f)
	{
		int idx = filters.indexOf(f);
		if (idx < 0)
			return;
		
		filters.removeAll(f);
		removeRow(idx);
	}
	
	void FilterList::filterEdited(Filter* f)
	{
		int idx = filters.indexOf(f);
		if (idx < 0)
			return;
		
		emit dataChanged(index(idx,0),index(idx,0));
	}
	
	void FilterList::saveFilters(const QString & file)
	{
		File fptr;
		if (!fptr.open(file,"wt"))
		{
			Out(SYS_SYN|LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
			return;
		}
		
		BEncoder enc(&fptr);
		enc.beginList();
		foreach (Filter* f,filters)
			f->save(enc);
		enc.end();
	}
	
	void FilterList::loadFilters(const QString & file)
	{
		QFile fptr(file);
		if (!fptr.open(QIODevice::ReadOnly))
		{
			Out(SYS_SYN|LOG_DEBUG) << "Failed to open " << file << " : " << fptr.errorString() << endl;
			return;
		}
		
		QByteArray data = fptr.readAll();
		BDecoder dec(data,false);
		BNode* n = 0;
		try
		{
			n = dec.decode();
			if (!n || n->getType() != BNode::LIST)
			{
				delete n;
				return;
			}
			
			BListNode* ln = (BListNode*)n;
			for (Uint32 i = 0;i < ln->getNumChildren();i++)
			{
				BDictNode* dict = ln->getDict(i);
				if (dict)
				{
					Filter* filter = new Filter();
					if (filter->load(dict))
						addFilter(filter);
					else 
						delete filter;
				}
			}
		}
		catch (bt::Error & err)
		{
			Out(SYS_SYN|LOG_DEBUG) << "Failed to parse " << file << " : " << err.toString() << endl;
		}
		
		delete n;
	}
	
	Filter* FilterList::filterForIndex(const QModelIndex & idx)
	{
		if (!idx.isValid())
			return 0;
		
		return filters.at(idx.row());
	}

	int FilterList::rowCount(const QModelIndex & parent) const
	{
		if (parent.isValid())
			return 0;
		else
			return filters.count();
	}
	
	QVariant FilterList::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid())
			return QVariant();
		
		Filter* f = filters.at(index.row());
		if (!f)
			return QVariant();
		
		switch (role)
		{
			case Qt::DisplayRole:
				return f->filterName();
			case Qt::DecorationRole:
				return KIcon("view-filter");
		}
			
		return QVariant();
	}
	
	bool FilterList::removeRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		endRemoveRows();
		return true;
	}
	
	bool FilterList::insertRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
}
