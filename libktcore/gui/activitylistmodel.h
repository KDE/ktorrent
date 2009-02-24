/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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

#ifndef ACTIVITYLISTMODEL_H
#define ACTIVITYLISTMODEL_H

#include <QStringListModel>

namespace kt
{
	class Activity;
	class ActivityListWidget;
	
	/// Model for the ActivityList
	class ActivityListModel : public QStringListModel
	{
		Q_OBJECT
	public:
		ActivityListModel(ActivityListWidget* widget);
		virtual ~ActivityListModel();
		
		void addActivity(Activity* act);
		void removeActivity(Activity* act);
		QModelIndex indexOf(Activity* act) const;
		void emitLayoutChanged();
		
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QVariant data(const QModelIndex& index, int role) const;
		virtual QModelIndex index(int row,int column,const QModelIndex & parent = QModelIndex()) const;
		virtual void sort(int column,Qt::SortOrder order);
	private:
		ActivityListWidget* widget;
		QList<Activity*> activities;
	};
}

#endif // ACTIVITYLISTMODEL_H
