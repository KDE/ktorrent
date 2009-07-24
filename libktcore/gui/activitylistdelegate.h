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

#ifndef ACTIVITYLISTDELEGATE_H
#define ACTIVITYLISTDELEGATE_H

#include <QStyledItemDelegate>

namespace kt
{

	class ActivityListDelegate : public QStyledItemDelegate
	{
	public:
		ActivityListDelegate(int icon_size,QObject* parent);
		virtual ~ActivityListDelegate();
		
		virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
		virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
		void setIconSize(int size) {icon_size = size;}
		void setShowIcons(bool icons) {show_icons = icons;}
		void setVertical(bool on) {vertical = on;}
	private:
		int icon_size;
		bool show_icons;
		bool vertical;
	};
}

#endif // ACTIVITYLISTDELEGATE_H
