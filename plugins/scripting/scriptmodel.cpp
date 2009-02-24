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
#include <klocale.h>
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
		// make sure we don't add dupes
		foreach (Script* s,scripts)
			if (s->scriptFile() == file)
				return;
		
		Script* s = new Script(file,this);
		scripts.append(s);
		insertRow(scripts.count());
	}
	
	Script* ScriptModel::addScriptFromDesktopFile(const QString & dir,const QString & desktop_file)
	{
		Script* s = new Script(this);
		if (!s->loadFromDesktopFile(dir,desktop_file))
		{
			delete s;
			return 0;
		}
		
		// we don't want dupes
		foreach (Script* os,scripts)
		{
			if (s->scriptFile() == os->scriptFile())
			{
				delete s;
				return 0;
			}
		}
		
		scripts.append(s);
		insertRow(scripts.count());
		return s;
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
				return s->iconName();
			case Qt::CheckStateRole:
				return s->running();
			case Qt::ToolTipRole:
				return i18n("<b>%1</b><br/><br/>%2",s->name(),s->metaInfo().comment);
			case CommentRole:
				return s->metaInfo().comment;
			case ConfigurableRole:
				return s->running() && s->hasConfigure();
			default:
				return QVariant();
		}
	}
	
	bool ScriptModel::setData(const QModelIndex & index,const QVariant & value,int role)
	{
		if (!index.isValid())
			return false;
		
		Script* s = scriptForIndex(index);
		if (!s)
			return false;
		
		if (role == Qt::CheckStateRole)
		{
			if (value.toBool())
				s->execute();
			else
				s->stop();
			 
			dataChanged(index,index);
			return true;
		}
		else if (role == ConfigureRole)
		{
			s->configure();
			return true;
		}
		else if (role == AboutRole)
		{
			showPropertiesDialog(s);
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
		Q_UNUSED(parent);
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		endRemoveRows();
		return true;
	}
	
	bool ScriptModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
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
	
	QStringList ScriptModel::runningScriptFiles() const
	{
		QStringList ret;
		foreach (Script* s,scripts)
		{
			if (s->running())
				ret << s->scriptFile();
		}
		return ret;
	}
	
	void ScriptModel::runScripts(const QStringList & r)
	{
		int idx = 0;
		foreach (Script* s,scripts)
		{
			if (r.contains(s->scriptFile()) && !s->running())
			{
				s->execute();
				QModelIndex i = index(idx,0);
				emit dataChanged(i,i);
			}
			idx++;
		}
	}
	
	void ScriptModel::removeScripts(const QModelIndexList & indices)
	{
		QList<Script*> to_remove;
		
		foreach (const QModelIndex & idx,indices)
		{
			Script* s = scriptForIndex(idx);
			if (s && s->removeable())
				to_remove << s;
		}
		
		foreach (Script* s,to_remove)
		{
			scripts.removeAll(s);
			s->stop();
			s->deleteLater();
		}
		
		reset();
	}
}
