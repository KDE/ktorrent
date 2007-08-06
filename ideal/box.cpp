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

#include "box.h"

namespace ideal
{
	
	Box::Box(bool vertical,QWidget* parent) : QWidget(parent)
	{
		if (vertical)
			box_layout = new QVBoxLayout(this);
		else
			box_layout = new QHBoxLayout(this);

		box_layout->setMargin(0);
		box_layout->setSpacing(0);
	}
	
	Box::~Box()
	{
	}

	void Box::addWidget(QWidget* widget)
	{
		box_layout->addWidget(widget);
	}
	
	void Box::insertWidget(QWidget* widget,int pos)
	{
		box_layout->insertWidget(pos,widget);
	}
}

#include "box.moc"

