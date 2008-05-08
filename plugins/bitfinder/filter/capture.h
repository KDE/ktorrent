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

#ifndef KTCAPTURE_H
#define KTCAPTURE_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QReadWriteLock>

namespace kt
	{
	
	class Capture : public QObject
		{
		Q_OBJECT
		
		public:
			Capture(QObject * parent = 0);
			Capture(const Capture& other);
			Capture& operator=(const Capture& other);
			~Capture() {}
			
			bool meetsMin(const Capture& min) const;
			bool meetsMax(const Capture& max) const;
			bool isEqual(const Capture& other) const;
			bool isEmpty() const;
			
			QPair<QString,QString> getVariable(int index) const;
			QString getValue(QString varName) const;
			QList< QPair<QString,QString> > getVariables() const;
			int varCount() const;
		
		public slots:
			void addVariable(QString name, QString value);
			
		private:
			mutable QReadWriteLock lock;
			QList< QPair<QString,QString> > variables;
		};
	
	}

#endif
