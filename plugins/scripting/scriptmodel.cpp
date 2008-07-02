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
#include "script.h"

namespace kt
{

	ScriptModel::ScriptModel(QObject* parent)
			: QAbstractListModel(parent)
	{
	}


	ScriptModel::~ScriptModel()
	{
	}
	
	void ScriptModel::addScript(const QString & file)
	{
		Script* s = new Script(file,this);
		scripts.append(s);
		insertRow(scripts.count());
	}
	
	int ScriptModel::rowCount(const QModelIndex & parent) const
	{
		return parent.isValid() ? 0 : scripts.count();
	}
	
	QVariant ScriptModel::data(const QModelIndex & index, int role) const
	{
		Script* s = scriptForIndex(index);
		if (!s)
			return QVariant();
		
		switch (role)
		{
			case Qt::DisplayRole:
				return s->name();
			case Qt::DecorationRole:
				return KIcon(s->iconName());
			case Qt::CheckStateRole:
				return s->running() ? Qt::Checked : Qt::Unchecked;
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
			Script* s = scriptForIndex(index);
			if (!s)
				return false;
			
			if ((Qt::CheckState)value.toUInt() == Qt::Checked)
				s->execute();
			else
				s->stop();
			 
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

	Script* ScriptModel::scriptForIndex(const QModelIndex & index) const
	{
		if (!index.isValid())
			return 0;
		
		if (index.row() < 0 || index.row() >= scripts.count())
			return 0;
			
		return scripts[index.row()];
	}
	
	bool ScriptModel::removeRows(int row,int count,const QModelIndex & parent)
	{
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		endRemoveRows();
		return true;
	}
	
	bool ScriptModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	QStringList ScriptModel::scriptFiles() const
	{
		QStringList ret;
		foreach (Script* s,scripts)
			ret << s->scriptFile();
		return ret;
	}
	
	void ScriptModel::removeScripts(const QModelIndexList & indices)
	{
		QList<Script*> to_remove;
		
		foreach (const QModelIndex & idx,indices)
			to_remove << scriptForIndex(idx);
		
		foreach (Script* s,to_remove)
		{
			scripts.removeAll(s);
			s->stop();
			s->deleteLater();
		}
		
		reset();
	}
}
