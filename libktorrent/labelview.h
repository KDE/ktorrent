/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
#ifndef KTLABELVIEW_H
#define KTLABELVIEW_H

#include <list>
#include <qscrollview.h>
#include "labelviewitembase.h"

class QLabel;
class QHBoxLayout;
class QVBoxLayout;

namespace kt
{
	class LabelView;
	
	/**
		Item in a LabelView
	*/
	class LabelViewItem : public LabelViewItemBase
	{
		Q_OBJECT
	public:
		LabelViewItem(const QString & icon,const QString & title,const QString & description,LabelView* view);
		virtual ~LabelViewItem();
		
		/// Set the title of the item
		void setTitle(const QString & title);
		
		/// Set the description
		void setDescription(const QString & d);
		
		/// Set the name of the icon
		void setIcon(const QString & icon);
		
		/// Set if this is an odd item (they have a different background color)
		void setOdd(bool odd);
		
		/// Set if this item is selected
		void setSelected(bool sel);
		
		/// Can be reimplemented to update the GUI of the item by base classes
		virtual void update() {}
		
		/// Smaller then operator for sorting (by default we sort on title)
		virtual bool operator < (const LabelViewItem & item);
		
	private:
		virtual void mousePressEvent(QMouseEvent *e);
		
	signals:
		void clicked(LabelViewItem* item);
		
	private:
		bool odd;
		bool selected;
	};
	
	class LabelViewBox;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class LabelView : public QScrollView
	{
		Q_OBJECT
	public:
		LabelView(QWidget *parent = 0, const char *name = 0);
		virtual ~LabelView();
		
		/// Add an item to the label view
		void addItem(LabelViewItem* item);
		
		/// Remove an item from the label view
		void removeItem(LabelViewItem* item);
		
		/// Get the current selected item (0 if none is selected)
		LabelViewItem* selectedItem() {return selected;}
		
		/// Clear the view
		void clear();
		
		/// Update all items in the view
		void update();
		
		/// Sort the items using the operator < 
		void sort();
	
	private slots:
		void onItemClicked(LabelViewItem* it);
		
	private:
		void updateOddStatus();
		
	signals:
		/// The current item has changed
		void currentChanged(LabelViewItem* item);
		
	private:
		LabelViewBox* item_box;
		std::list<LabelViewItem*> items;
		LabelViewItem* selected;
	};

}

#endif
