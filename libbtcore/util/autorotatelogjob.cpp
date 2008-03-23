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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "autorotatelogjob.h"
#include <kurl.h>
#include <k3process.h>
#include <util/fileops.h>
#include "log.h"

namespace bt
{

	AutoRotateLogJob::AutoRotateLogJob(const QString & file,Log* lg)
		: KIO::Job(),file(file),cnt(10),lg(lg)
	{
		update();
	}


	AutoRotateLogJob::~AutoRotateLogJob()
	{}

	void AutoRotateLogJob::kill(bool)
	{
		emitResult();
	}
		
	void AutoRotateLogJob::update()
	{
		while (cnt > 1)
		{
			QString prev = QString("%1-%2.gz").arg(file).arg(cnt - 1);
			QString curr = QString("%1-%2.gz").arg(file).arg(cnt);
			if (bt::Exists(prev)) // if file exists start the move job
			{
				KIO::Job* sj = KIO::file_move(KUrl(prev),KUrl(curr),-1, KIO::Overwrite | KIO::HideProgressInfo);
				connect(sj,SIGNAL(result(KJob*)),this,SLOT(moveJobDone(KJob* )));	
				return;
			}
			else
			{
				cnt--;
			}
		}
			
		if (cnt == 1)
		{
			// move current log to 1 and zip it
			KIO::Job* sj = KIO::file_move(KUrl(file),KUrl(file + "-1"),-1, KIO::Overwrite | KIO::HideProgressInfo);
			connect(sj,SIGNAL(result(KJob*)),this,SLOT(moveJobDone(KJob* )));
		}
		else
		{
				// final log file is moved, now zip it and end the job
			std::system(QString("gzip " + K3Process::quote(file + "-1")).toLocal8Bit());
			lg->logRotateDone();
			emitResult();
		}
	}
		
	
	void AutoRotateLogJob::moveJobDone(KJob*)
	{
		cnt--; // decrease counter so the newt file will be moved in update
		update(); // don't care about result of job
	}

}
#include "autorotatelogjob.moc"
