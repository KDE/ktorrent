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

#include <QListView>
#include <KSharedConfig>

class KMenu;
class QAction;

namespace kt
{
	class Activity;
	class ActivityListModel;
	class ActivityListDelegate;
	
	enum ActivityListDisplayMode
	{
		ICONS_AND_TEXT,
		ICONS_ONLY,
		TEXT_ONLY
	};
	
	
	/**
	 * List widget to display the activity list.
	 */
	class ActivityListWidget : public QListView
	{
		Q_OBJECT
	public:
		ActivityListWidget(QWidget* parent);
		virtual ~ActivityListWidget();
		
		ActivityListDisplayMode displayMode() const {return mode;}
		void addActivity(Activity* a);
		void removeActivity(Activity* act);
		void setCurrentActivity(Activity* act);
		void loadState(KSharedConfigPtr cfg);
		void saveState(KSharedConfigPtr cfg);
		
		virtual QSize sizeHint() const;
	private slots:
		void showConfigMenu(QPoint pos);
		void iconSizeActionTriggered(QAction* act);
		void modeActionTriggered(QAction* act);
		void updateParentSize();
		void currentItemChanged(const QModelIndex & sel,const QModelIndex & old);
		
	signals:
		void currentActivityChanged(Activity* act);
		
	private:
		void showEvent(QShowEvent* event);
		
	private:
		KMenu* menu;
		QAction* little_icons;
		QAction* normal_icons;
		QAction* big_icons;
		QAction* show_icons_only;
		QAction* show_text_only;
		QAction* show_icons_and_text;
		ActivityListModel* model;
		ActivityListDelegate* delegate;
		int icon_size;
		ActivityListDisplayMode mode;
	};
}

#endif // ACTIVITYLISTWIDGET_H
