/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
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
#include <kio/job.h>
#include "feedretriever.h"

namespace kt
{

	FeedRetriever::FeedRetriever(const QString & file_name) : fptr(file_name),job(0),err(0)
	{
	}
	
	
	FeedRetriever::~FeedRetriever()
	{
	}
	
	void FeedRetriever::abort()
	{
	}
	
	int FeedRetriever::errorCode() const
	{
		return err;
	}
	
	void FeedRetriever::retrieveData(const KUrl &url)
	{
		job = KIO::storedGet(url,KIO::Reload,KIO::HideProgressInfo);
		connect(job,SIGNAL(result(KJob*)),this,SLOT(finished(KJob*)));
	}

	void FeedRetriever::finished(KJob* j)
	{
		KIO::StoredTransferJob* stj = (KIO::StoredTransferJob*)j;
		err = stj->error();
		QByteArray data = stj->data();
		if (!err && fptr.open(QIODevice::WriteOnly))
		{
			fptr.write(data);
			fptr.close();
		}
		dataRetrieved(data,err == 0);
	}

}
