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

#ifndef KTMATCHES_H
#define KTMATCHES_H

#include <QAbstractItemModel>
#include <QReadWriteLock>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QTimer>

#include <util/constants.h>

#include "bfitem.h"
#include "capture.h"

namespace kt
{
	class Matches : public QAbstractItemModel
	{
		Q_OBJECT
	public:
		Matches(const QString& baseDir);
		~Matches() {}
		
		virtual int rowCount(const QModelIndex & parent) const;
		virtual int columnCount(const QModelIndex & parent) const;
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		virtual QModelIndex index(int row,int column,const QModelIndex & parent) const;
		virtual QModelIndex parent(const QModelIndex & index) const;
		
		int captureCount(Capture * capture, const QStringList& releaseTerms = QStringList()) const;
		
	public slots:
		void unload();
		void resetChangeTimer();
		void saveMatches();
		
		void addColumn(const QString& name);
		void addMatch(BFItem * item, Capture * capture);
		
	signals:
		
		
	private:
		mutable QReadWriteLock lock;
		QTimer changeTimeout;
		
		bool loadMatches();
		
		QString configDirName;
		
		QDomDocument matchXml;
		QDomElement root;
		
		QStringList columnNames;
	};

}

#endif
