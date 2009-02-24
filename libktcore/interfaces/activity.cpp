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

#include <KStringHandler>
#include "activity.h"

namespace kt
{
	Activity::Activity(const QString& name, const QString& icon, int weight, QWidget* parent) 
		: QWidget(parent),activity_name(name),activity_icon(icon),activity_weight(weight)
	{
	}

	Activity::~Activity() 
	{
	}
	
	void Activity::setName(const QString& name) 
	{
		activity_name = name;
		nameChanged(this,name);
	}

	void Activity::setIcon(const QString& icon) 
	{
		activity_icon = icon;
		iconChanged(this,icon);
	}

	bool Activity::lessThan(Activity* l,Activity* r)
	{
		if (l->weight() == r->weight())
			return KStringHandler::naturalCompare(l->name(),r->name()) < 0;
		else
			return l->weight() < r->weight();
	}
}