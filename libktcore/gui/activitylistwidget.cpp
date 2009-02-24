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
#include <QMouseEvent>
#include <KIcon>
#include <interfaces/activity.h>
#include "activitylistwidget.h"
#include "activitylistdelegate.h"


namespace kt
{
	ActivityListWidget::ActivityListWidget(QWidget* parent) : QListWidget(parent)
	{
		setItemDelegate(new ActivityListDelegate(this));
		setMouseTracking(true);
		viewport()->setAttribute(Qt::WA_Hover);
		setSelectionMode(QAbstractItemView::SingleSelection);
		int iconsize = 48; // TODO: SETTINGS
		setIconSize(QSize(iconsize, iconsize));
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setContextMenuPolicy(Qt::CustomContextMenu);
		QPalette pal = palette();
		pal.setBrush(QPalette::Base, pal.brush(QPalette::Window));
		setPalette(pal);
	}

	ActivityListWidget::~ActivityListWidget() 
	{
	}
	
	void ActivityListWidget::mouseDoubleClickEvent(QMouseEvent* event)
	{
		QModelIndex index = indexAt(event->pos());
		if (index.isValid() && !(index.flags() & Qt::ItemIsSelectable))
			return;
		
		QListWidget::mouseDoubleClickEvent(event);
	}
	
	void ActivityListWidget::mouseMoveEvent(QMouseEvent* event)
	{
		QModelIndex index = indexAt(event->pos());
		if (index.isValid() && !(index.flags() & Qt::ItemIsSelectable))
			return;
		
		QListWidget::mouseMoveEvent(event);
	}
	
	void ActivityListWidget::mousePressEvent(QMouseEvent* event)
	{
		QModelIndex index = indexAt(event->pos());
		if (index.isValid() && !(index.flags() & Qt::ItemIsSelectable))
			return;
		
		QListWidget::mousePressEvent(event);
	}
	
	void ActivityListWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		QModelIndex index = indexAt(event->pos());
		if (index.isValid() && !(index.flags() & Qt::ItemIsSelectable))
			return;
		
		QListWidget::mouseReleaseEvent(event);
	}
	
#if 0
	QModelIndex ActivityListWidget::moveCursor(QAbstractItemView::CursorAction cursorAction,Qt::KeyboardModifiers modifiers)
	{
		Q_UNUSED(modifiers)
		QModelIndex oldindex = currentIndex();
		QModelIndex newindex = oldindex;
		switch ( cursorAction )
		{
			case MoveUp:
			case MovePrevious:
			{
				int row = oldindex.row() - 1;
				while (row > -1 && !(model()->index(row, 0).flags() & Qt::ItemIsSelectable)) 
					--row;
				if (row > -1)
					newindex = model()->index(row, 0);
				break;
			}
			case MoveDown:
			case MoveNext:
			{
				int row = oldindex.row() + 1;
				int max = model()->rowCount();
				while (row < max && !(model()->index(row,0).flags() & Qt::ItemIsSelectable))
					++row;
				if (row < max)
					newindex = model()->index(row,0);
				break;
			}
			case MoveHome:
			case MovePageUp:
			{
				int row = 0;
				while (row < oldindex.row() && !(model()->index(row,0).flags() & Qt::ItemIsSelectable)) 
					++row;
				if (row < oldindex.row())
					newindex = model()->index(row,0);
				break;
			}
			case MoveEnd:
			case MovePageDown:
			{
				int row = model()->rowCount() - 1;
				while (row > oldindex.row() && !(model()->index(row,0).flags() & Qt::ItemIsSelectable)) 
					--row;
				if (row > oldindex.row())
					newindex = model()->index(row,0);
				break;
			}
			// no navigation possible for these
			case MoveLeft:
			case MoveRight:
				break;
		}
		
		// dirty hack to change item when the key cursor changes item
		if (oldindex != newindex)
		{
			emit itemClicked(itemFromIndex(newindex));
		}
		return newindex;
	}
#endif

	void ActivityListWidget::addActivity(Activity* a)
	{
		ActivityListWidgetItem* item = new ActivityListWidgetItem(a);
		addItem(item);
	}
	
	void ActivityListWidget::removeActivity(int idx)
	{
		QListWidgetItem* it = item(idx);
		delete it;
	}
	
	ActivityListWidgetItem::ActivityListWidgetItem(Activity* a) : QListWidgetItem(0,ActivityListWidgetItemType),activity(a)
	{
		setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		setIcon(KIcon(activity->icon()));
		setText(activity->name());
		setToolTip(activity->toolTip());
	}

}
