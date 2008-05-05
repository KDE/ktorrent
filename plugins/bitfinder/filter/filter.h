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

#ifndef KTFILTER_H
#define KTFILTER_H

#include <QThread>
#include <QString>
#include <QStringList>
#include <QReadWriteLock>
#include <util/constants.h>

#include "filterconstants.h"

namespace kt
{
	class Filter : public QThread
	{
		Q_OBJECT
	public:
		Filter();
		~Filter();
		
		QString getName();
		QString getIconName();
		int getType();
		QString getGroup();
		QStringList getExpressions();
		int getSourceListType();
		QStringList getSourceList();
		int getMultiMatch();
		int getRerelease();
		
	public slots:
		void setName(const QString& value);
		void setType(int value);
		void setGroup(const QString& value);
		void setExpressions(QStringList value);
		void setSourceListType(int value);
		void setSourceList(QStringList value);
		void setMultiMatch(int value);
		void setRerelease(int value);
	
	signals:
		void changed();
		void nameChanged(const QString& name);
		void typeChanged(int type);
		void groupChanged(const QString& group);
		void expressionsChanged(QStringList expressions);
		void sourceListTypeChanged(int sourceListType);
		void sourceListChanged(QStringList expressions);
		void multiMatchChanged(int multiMatch);
		void rereleaseChanged(int rerelease);
		
	protected:
		virtual void run();
	
	private:
		QReadWriteLock lock;
		QString name;
		int type;
		QString group;
		QStringList expressions;
		int multiMatch;
		int rerelease;
		QStringList captureExpressions;
		//add captureCheckList
		int sourceListType;
		QStringList sourceList;
		
	};

}

#endif
