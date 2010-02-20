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
#include <util/log.h>
#include <util/functions.h>
#include "net/wakeuppipe.h"

using namespace bt;

namespace net
{

	WakeUpPipe::WakeUpPipe() : woken_up(false)
	{
	}


	WakeUpPipe::~WakeUpPipe()
	{
	}

	void WakeUpPipe::wakeUp()
	{
		QMutexLocker lock(&mutex);
		if (woken_up)
			return;
		
		char dummy[] = "dummy";
		if (bt::Pipe::write((const bt::Uint8*)dummy,5) != 5)
			Out(SYS_GEN|LOG_DEBUG) << "WakeUpPipe: wake up failed " << endl;
		else
			woken_up = true;
	}
		
	void WakeUpPipe::handleData()
	{
		QMutexLocker lock(&mutex);
		bt::Uint8 buf[20];
		int ret = bt::Pipe::read(buf,20);
		if (ret < 0)
			Out(SYS_GEN|LOG_DEBUG) << "WakeUpPipe: read failed " << endl;
		
		woken_up = false;
	}
	
	
	void WakeUpPipe::reset()
	{
		// Do nothing
	}

}
