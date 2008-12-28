/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#ifndef KTLOGFLAGS_H
#define KTLOGFLAGS_H

#include <QList>
#include <QAbstractTableModel>
#include <util/constants.h>

class QString;

namespace kt
{
	
	
	/**
	 * Class to read/save logging messages flags.
	 * @author Ivan Vasic <ivasic@gmail.com>
	*/
	class LogFlags : public QAbstractTableModel
	{
		Q_OBJECT
				
	public:
		LogFlags();
		virtual ~LogFlags();
		
		static LogFlags& instance();
		
		///Checks current flags with arg. Return true if message should be shown
		bool checkFlags(unsigned int arg);
		
		///Updates flags from Settings::
		void updateFlags();
		
		///Makes line rich text according to arg level.
		QString& getFormattedMessage(unsigned int arg, QString& line);
		
		virtual int rowCount(const QModelIndex & parent) const;
		virtual int columnCount(const QModelIndex & parent) const;
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		virtual bool setData(const QModelIndex & index,const QVariant & value,int role);
		virtual Qt::ItemFlags flags(const QModelIndex & index) const;
		virtual bool removeRows(int row,int count,const QModelIndex & parent);
		virtual bool insertRows(int row,int count,const QModelIndex & parent);
		
	private slots:
		void registered(const QString & sys);
		void unregistered(const QString & sys);
		
	private:
		QString flagToString(bt::Uint32 flag) const;
			
	private:
		struct LogFlag
		{
			QString name;
			bt::Uint32 id;
			bt::Uint32 flag;
		};
		QList<LogFlag> log_flags;
	};

}

#endif
