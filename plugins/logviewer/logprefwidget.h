/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasic                                      *
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
#ifndef LOGPREFWIDGET_H
#define LOGPREFWIDGET_H

#include "logprefwidgetbase.h"

namespace kt
{
	class LogPrefWidget: public LogPrefWidgetBase
	{
			Q_OBJECT
		public:
			LogPrefWidget(QWidget *parent = 0, const char *name = 0);
			bool apply();
			
		private:
			int getLevel(unsigned int arg);
			unsigned int getArg(int level);
	};
}
#endif
