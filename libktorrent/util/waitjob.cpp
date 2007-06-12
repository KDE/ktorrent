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
#include <torrent/globals.h>
#include <kio/netaccess.h>
#include "waitjob.h"
#include "log.h"

namespace bt
{

	WaitJob::WaitJob(Uint32 millis) : KIO::Job(false)
	{
		connect(&timer,SIGNAL(timeout()),this,SLOT(timerDone()));
		timer.start(millis,true);
	}


	WaitJob::~WaitJob()
	{}

	void WaitJob::kill(bool)
	{
		m_error = 0;
		emitResult();
	}
		
	void WaitJob::timerDone()
	{
		// set the error to null and emit the result
		m_error = 0;
		emitResult();
	}
	
	void WaitJob::addExitOperation(kt::ExitOperation* op)
	{
		exit_ops.append(op);
		connect(op,SIGNAL(operationFinished( kt::ExitOperation* )),
				this,SLOT(operationFinished( kt::ExitOperation* )));
	}
	
	void WaitJob::operationFinished(kt::ExitOperation* op)
	{
		if (exit_ops.count() > 0)
		{
			exit_ops.remove(op);
			if (op->deleteAllowed())
				op->deleteLater();
			
			if (exit_ops.count() == 0)
				timerDone();
		}
	}
	
	void WaitJob::execute(WaitJob* job)
	{
		KIO::NetAccess::synchronousRun(job,0);
	}
	
	void SynchronousWait(Uint32 millis)
	{
		Out() << "SynchronousWait" << endl;
		WaitJob* j = new WaitJob(millis);
		KIO::NetAccess::synchronousRun(j,0);
	}

}

#include "waitjob.moc"

