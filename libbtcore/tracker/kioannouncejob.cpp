/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#include "kioannouncejob.h"
#include <kio/job.h>
#include <util/log.h>

namespace bt
{
	
	KIOAnnounceJob::KIOAnnounceJob(const KUrl& url,const KIO::MetaData & md) : url(url)
	{
		get_job = KIO::get(url,KIO::NoReload,KIO::HideProgressInfo);
		get_job->setMetaData(md);
		connect(get_job,SIGNAL(data(KIO::Job*,QByteArray)),this,SLOT(data(KIO::Job*,QByteArray)));
		connect(get_job,SIGNAL(result(KJob*)),this,SLOT(finished(KJob*)));
	}

	KIOAnnounceJob::~KIOAnnounceJob()
	{
	}

	void KIOAnnounceJob::data(KIO::Job* j, const QByteArray& data)
	{
		const int MAX_REPLY_SIZE = 1024 * 1024;
		Q_UNUSED(j);
		if (reply_data.size() + data.size() > MAX_REPLY_SIZE)
		{
			// If the reply is larger then a mega byte, the server
			// has probably gone bonkers
			get_job->kill();
			setError(KIO::ERR_ABORTED);
			Out(SYS_TRK|LOG_DEBUG) << "Tracker sending back to much data in announce reply, aborting ..." << endl;
			emitResult();
		}
		else
			reply_data.append(data);
	}
	
	bool KIOAnnounceJob::doKill()
	{
		get_job->kill();
		return KIO::Job::doKill();
	}
	
	void KIOAnnounceJob::finished(KJob* j)
	{
		setError(j->error());
		emitResult();
	}
}

