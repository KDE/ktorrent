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
#include <kicon.h>
#include "scriptmodel.h"

namespace kt
{

	ScriptModel::ScriptModel(Kross::ActionCollection* col,QObject* parent)
			: QAbstractListModel(parent),col(col)
	{
		connect(col,SIGNAL(updated()),this,SLOT(collectionUpdated()));
	}


	ScriptModel::~ScriptModel()
	{
	}
	
	void ScriptModel::collectionUpdated()
	{
		reset();
	}
		
	int ScriptModel::rowCount(const QModelIndex & parent) const
	{
		return parent.isValid() ? 0 : col->actions().count();
	}
	
	QVariant ScriptModel::data(const QModelIndex & index, int role) const
	{
		if (!index.isValid())
			return QVariant();
		
		QList<Kross::Action*> act = col->actions();
		if (index.row() < 0 || index.row() >= act.count())
			return QVariant();
			
		Kross::Action* a = act[index.row()];
		switch (role)
		{
			case Qt::DisplayRole:
				return a->name();
			case Qt::DecorationRole:
				return KIcon(a->iconName());
			case Qt::CheckStateRole:
				return a->isEnabled() ? Qt::Checked : Qt::Unchecked;
			default:
				return QVariant();
		}
	}
	
	bool ScriptModel::setData(const QModelIndex & index,const QVariant & value,int role)
	{
		if (!index.isValid())
			return false;
		
		if (role == Qt::CheckStateRole)
		{
			QList<Kross::Action*> act = col->actions();
			if (index.row() < 0 || index.row() >= act.count())
				return false;
			
			Kross::Action* a = act[index.row()];
			a->setEnabled((Qt::CheckState)value.toUInt() == Qt::Checked);
			if ((Qt::CheckState)value.toUInt() == Qt::Checked)
				a->trigger(); 
			dataChanged(index,index);
			return true;
		}
		return false;
	}
	
	Qt::ItemFlags ScriptModel::flags(const QModelIndex & index) const
	{
		if (!index.isValid())
			return QAbstractItemModel::flags(index);
		else
			return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
	}

}
