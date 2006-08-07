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
#ifndef BTWAITJOB_H
#define BTWAITJOB_H

#include <qtimer.h>
#include <kio/job.h>
#include "constants.h"

namespace bt
{

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	
		Job to wait for a certain amount of time.
	*/
	class WaitJob : public KIO::Job
	{
		Q_OBJECT
	public:
		WaitJob(Uint32 millis);
		virtual ~WaitJob();

		virtual void kill(bool quietly=true);
		
	private slots:
		void timerDone();
		
	private:
		QTimer timer;
	};
	
	void SynchronousWait(Uint32 millis);

}

#endif
