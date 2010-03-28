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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTTIMER_H
#define BTTIMER_H

#include <util/log.h>
#include <util/constants.h>
#include <btcore_export.h>
#include "constants.h"

namespace bt
{

	/**
	@author Joris Guisson
	*/
	class BTCORE_EXPORT Timer
	{
		TimeStamp last;
		TimeStamp elapsed;
	public:
		Timer();
		Timer(const Timer & t);
		virtual ~Timer();

		TimeStamp getLast() const {return last;}
		TimeStamp update();
		TimeStamp getElapsed() const {return elapsed;}
		TimeStamp getElapsedSinceUpdate() const;
		Timer & operator = (const Timer & t);
	};
#if 1
	class Marker
	{
		QString name;
		Timer timer;
		public:
			Marker(const QString & name) : name(name) 
			{
				timer.update();
			}
		
			void update()
			{
				timer.update();
				Out(SYS_GEN|LOG_DEBUG) << "Mark: " << name << " : " << timer.getElapsed() << " ms" << endl;
			}
	};
#endif
}

#endif
