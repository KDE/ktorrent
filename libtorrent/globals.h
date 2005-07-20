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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTGLOBALS_H
#define BTGLOBALS_H

#include <libutil/constants.h>

class QString;

namespace bt
{
	class Log;


	class Globals
	{
	public:
		virtual ~Globals();
		
		void initLog(const QString & file);
		void setDebugMode(bool on) {debug_mode = on;}
		bool isDebugModeSet() const {return debug_mode;}

		Log & getLog() {return *log;}
		static Globals & instance() {return inst;}
	private:
		Globals();
		
		bool debug_mode;
		Log* log;

		static Globals inst;
		
		friend Log& Out();
	};

}

#endif
