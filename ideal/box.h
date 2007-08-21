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
#ifndef IDEAL_BOX_HHH
#define IDEAL_BOX_HHH

#include <QWidget>
#include <QBoxLayout>

namespace ideal
{
	/**
	 * Container widget which lays out child widgets in vertical or 
	 * horizontal direction
	 * */
	class Box : public QWidget
	{
		Q_OBJECT
	public:
		Box(bool vertical,QWidget* parent);
		virtual ~Box();

		/// Add a new widget to the box
		void addWidget(QWidget* widget);

		/// Insert a new widget at a certain position
		void insertWidget(QWidget* widget,int pos);
	private:
		QBoxLayout* box_layout;
	};
}

#endif
