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
#ifndef KTBFITEM_H
#define KTBFITEM_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QStringList>

namespace kt
	{
	
	class BFItem : public QObject
		{
			Q_OBJECT
		
		public:
			BFItem(const QString& name, const QString& source, const QString& link, 
				const QString& description, const QByteArray& torrentData, QObject * parent = 0);
			~BFItem() { }
		
			QString getName() const;
			QString getSource() const;
			QString getLink() const;
			QString getDescription() const;
			QByteArray getTorrentData() const;
			
			QStringList getFilenames() const;
			
			bool isValid() const;
		
		public slots:
		
		signals:
		
		private:
			//This class cannot be changed after creation. So we don't require any locks for thread safety
			const QString name;
			const QString source;
			const QString link;//this must be a string unique to this BFItem within the source it came from
			const QString description;
			const QByteArray torrentData;
		};
	
	}

#endif
