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
#ifndef KTSPINBOXDELEGATE_H
#define KTSPINBOXDELEGATE_H

#include <QItemDelegate>

namespace kt
{

	/**
		@author
	*/
	class SpinBoxDelegate : public QItemDelegate
	{
		Q_OBJECT

	public:
		SpinBoxDelegate(QObject *parent = 0);
		virtual ~SpinBoxDelegate();

		virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index) const;
		virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
		virtual void setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const;
		virtual void updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &index) const;
		virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
	};
}

#endif
