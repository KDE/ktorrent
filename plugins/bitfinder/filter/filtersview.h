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

#ifndef KTFILTERSVIEW_H
#define KTFILTERSVIEW_H

#include <kaction.h>

#include <QListView>
#include <QToolBar>

#include "filterlistmodel.h"

namespace kt
	{

	class FiltersView : public QWidget
		{
			Q_OBJECT
		public:
			FiltersView (FilterListModel * model, QWidget* parent = 0); //add in the model later
			~FiltersView() { }

			/// Get the media tool bar
			QToolBar* filtersToolBar() {return toolBar;}

			/// Get the current selected item
			//QModelIndex selectedItem() const;

		public slots:
			void addNewFilter();
			void removeFilters();
			
		private slots:
//			void onSelectionChanged (const QItemSelection & s, const QItemSelection & d);

		signals:
//			void selectionChanged (const QModelIndex & idx);
			void doubleClicked (const QModelIndex & idx);

		private:
			void setupFiltersActions();
			
			QToolBar* toolBar;
			QListView* filtersList;
			FilterListModel* filterListModel;
		
 			KAction * addFilter;
 			KAction * removeFilter;
 			KAction * filterUp;
 			KAction * filterDown;
		};


	}

#endif


