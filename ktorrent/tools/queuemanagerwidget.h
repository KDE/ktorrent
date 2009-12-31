/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#ifndef KTQUEUEMANAGERWIDGET_H
#define KTQUEUEMANAGERWIDGET_H

#include <QWidget>
#include <ksharedconfig.h>

class QModelIndex;
class QToolBar;
class QTreeView;

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class HintLineEdit;
	class QueueManager;
	class QueueManagerModel;
	
	/**
	 * @author Joris Guisson
	 * 
	 * Widget for the QueueManager
	*/
	class QueueManagerWidget : public QWidget
	{
		Q_OBJECT
	public:
		QueueManagerWidget(QueueManager* qman,QWidget* parent);
		virtual ~QueueManagerWidget();
		
		/// Save the widget state
		void saveState(KSharedConfigPtr cfg);
		/// Load the widget state
		void loadState(KSharedConfigPtr cfg);
		/// Update the widget
		void update();
		
	public slots:
		void onTorrentAdded(bt::TorrentInterface* tc);
		void onTorrentRemoved(bt::TorrentInterface* tc);
		
	private slots:
		void moveUpClicked();
		void moveDownClicked();
		void moveTopClicked();
		void moveBottomClicked();
		void searchTextChanged(const QString & t);
		void showSearch(bool on);

	private:
		bool indexVisible(const QModelIndex & idx);
		
	private:
		QueueManagerModel* model;
		QueueManager* qman;
		QTreeView* view;
		QToolBar* toolbar;
		HintLineEdit* search;
		
		QAction* show_search;
		QAction* move_top;
		QAction* move_up;
		QAction* move_down;
		QAction* move_bottom;
	};
}

#endif
