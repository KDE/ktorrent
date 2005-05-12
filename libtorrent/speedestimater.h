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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BTSPEEDESTIMATER_H
#define BTSPEEDESTIMATER_H


#include "globals.h"

namespace bt
{

	/**
	@author Joris Guisson
	*/
	class SpeedEstimater
	{
		class SpeedEstimaterPriv;
	public:
		SpeedEstimater();
		virtual ~SpeedEstimater();

		void onWrite(Uint32 bytes);
		void onRead(Uint32 bytes);
		void update();
		
		Uint32 downloadRate() const {return download_rate;}
		Uint32 uploadRate() const {return upload_rate;}
	private:
		Uint32 upload_rate,download_rate;
		SpeedEstimaterPriv* up;
		SpeedEstimaterPriv* down;
	};

}

#endif
