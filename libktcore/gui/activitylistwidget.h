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

#ifndef ACTIVITYLISTWIDGET_H
#define ACTIVITYLISTWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>

namespace kt
{
	class Activity;
	
	/**
	 * List widget to display the activity list.
	 */
	class ActivityListWidget : public QListWidget
	{
		Q_OBJECT
	public:
		ActivityListWidget(QWidget* parent);
		virtual ~ActivityListWidget();
		
		virtual void mouseDoubleClickEvent(QMouseEvent* event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void mousePressEvent(QMouseEvent* event);
		virtual void mouseReleaseEvent(QMouseEvent* event);
		//virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,Qt::KeyboardModifiers modifiers);
		
		void addActivity(Activity* a);
		void removeActivity(int idx);
	};
	
	const int ActivityListWidgetItemType = QListWidgetItem::UserType + 1;
	
	class ActivityListWidgetItem : public QListWidgetItem
	{
	public:
		ActivityListWidgetItem(Activity* a);
		
		Activity* activity;
	};
}

#endif // ACTIVITYLISTWIDGET_H
