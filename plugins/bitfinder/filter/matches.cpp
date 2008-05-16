/***************************************************************************
 *   Copyright (C) 2008 by Alan Jones                                      *
 *   skyphyr@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#include <util/log.h>

#include <QReadLocker>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>

#include "matches.h"

using namespace bt;

namespace kt
	{
	
	Matches::Matches(const QString& baseDir) : configDirName(baseDir), matchXml("BitFinderMatches")
		{
		//let's initialize the column list
		columnNames << "Name";
		
		if (!loadMatches())
			{
			root = matchXml.createElement("BitFinderMatches");
			matchXml.appendChild(root);
			}
		
		}
	
	int Matches::rowCount(const QModelIndex & parent) const
		{
		Q_UNUSED(parent);
		QReadLocker readLock(&lock);
		
		return root.elementsByTagName("Match").count();
		}
	
	int Matches::columnCount(const QModelIndex & parent) const
		{
		Q_UNUSED(parent);
		QReadLocker readLock(&lock);
		
		return columnNames.count();
		}
	
	QVariant Matches::headerData(int section, Qt::Orientation orientation,int role) const
		{
		QReadLocker readLock(&lock);
		
		if (orientation==Qt::Horizontal)
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (section <= 0 || section >= columnNames.count())
			return QVariant();
		
		//columnNames contains the title for each column
		return columnNames.at(section);
		}
	
	QVariant Matches::data(const QModelIndex & index, int role) const
		{
		QReadLocker readLock(&lock);
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (index.column() < 0 || index.column() >= columnNames.count())
			return QVariant();
		
		if (index.row() < 0 || index.row() >= root.elementsByTagName("Match").count())
			return QVariant();
		
		//we know it's something within the list - so let's find the data
		QDomElement curMatch = root.elementsByTagName("Match").at(index.row()).toElement();
		QDomElement curVar;
		
		for (int i=0; i<curMatch.elementsByTagName("Variable").count(); i++)
			{
			curVar = curMatch.elementsByTagName("Variable").at(i).toElement();
			if (curVar.hasAttribute("Name"))
				{
				if (columnNames.at(index.column()) == curVar.attribute("Name"))
					return curVar.attribute("Value");
				}
			}
		
		return QVariant();
		}
	
	QModelIndex Matches::index(int row,int column,const QModelIndex & parent) const
		{
		Q_UNUSED(parent);
		return createIndex(row, column);
		}
	
	QModelIndex Matches::parent(const QModelIndex & index) const
		{
		Q_UNUSED(index);
		return QModelIndex();
		}
	
	void Matches::addColumn(const QString& name)
		{
		QWriteLocker writeLock(&lock);
		
		if (columnNames.contains(name))
			return;
		
		beginInsertColumns(QModelIndex(), columnNames.count(), columnNames.count());
		columnNames.append(name);
		endInsertColumns();
		}
	
	void Matches::addMatch(BFItem * item, Capture * capture)
		{
		//there may be new Columns in here
		for (int i=0; i<capture->varCount(); i++)
			{
 			addColumn(capture->getVariable(i).first);
			}
		
		QWriteLocker writeLock(&lock);
		
		//this is a model so let's say we're adding the rows
		beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));
		
		QDomElement newMatch;
		newMatch.setAttribute("Name", item->getName());
		newMatch.setAttribute("Source", item->getSource());
		newMatch.setAttribute("Link", item->getLink());
		newMatch.setAttribute("Description", item->getDescription());
		
		for (int i=0; i<capture->varCount(); i++)
			{
			QDomElement curVar;
 			curVar.setAttribute("Name", capture->getVariable(i).first);
 			curVar.setAttribute("Value", capture->getVariable(i).second);
			}
		
		root.appendChild(newMatch);
		
		if (!item->getLink().isEmpty())
			{
			//the link seems ok - so let's save off the torrent data
			QFile torFile(configDirName + item->getLink() + ".torrent");
			torFile.open(QFile::WriteOnly);
			torFile.write(item->getTorrentData());
			torFile.close();
			}
		
		//and now we're adding the new Match
		endInsertRows();
		
		}
	
	bool Matches::loadMatches()
		{
		//let's verify the settings directory exists
		QFileInfo configDir(configDirName);
		if (configDir.exists())
			{
			if (!configDir.isDir())
				{
				//it's a file :O
				//delete the file, then create the directory
				QFile vigilantie(configDirName);
				vigilantie.remove();
				QDir mkConfigDir;
				mkConfigDir.mkdir(configDirName);
				}
			}
		else
			{
			//doesn't exist - let's create it
			QDir mkConfigDir;
			mkConfigDir.mkdir(configDirName);
			}
		
		QFile file(configDirName + "matches.xml");
		
		if (!file.open(QIODevice::ReadOnly))
			{
			Out(SYS_BTF|LOG_NOTICE) << "Failed to open match file " << configDirName << "matches.xml" << endl;
			return false;
			}
		
		if (!matchXml.setContent(&file))
			{
			Out(SYS_BTF|LOG_NOTICE) << "Failed to load XML from match file " << configDirName << "matches.xml" << endl;
			file.close();
			return false;
			}
		
		file.close();
		
		if (!matchXml.elementsByTagName("BitFinderMatches").count())
			return false;
		
		root = matchXml.elementsByTagName("BitFinderMatches").at(0).toElement();
		return true;
		}
	
	}
