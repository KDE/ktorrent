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
#include <ktversion.h>
#include <util/log.h>
#include "feedretriever.h"

using namespace bt;

namespace kt
{
    FeedRetriever::FeedRetriever() : job(0), err(0)
    {
    }

    FeedRetriever::FeedRetriever(const QString& file_name) : backup_file(file_name), job(0), err(0)
    {
    }


    FeedRetriever::~FeedRetriever()
    {
    }

    void FeedRetriever::setAuthenticationCookie(const QString& cookie)
    {
        this->cookie = cookie;
    }

    void FeedRetriever::abort()
    {
        if (job)
            job->kill(KJob::EmitResult);
    }

    int FeedRetriever::errorCode() const
    {
        return err;
    }

    void FeedRetriever::retrieveData(const QUrl& url)
    {
        KIO::StoredTransferJob* j = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
        j->addMetaData("UserAgent", bt::GetVersionString());
        if (!cookie.isEmpty())
        {
            j->addMetaData("cookies", "none");
            j->addMetaData("customHTTPHeader", QString("Cookie: %1").arg(cookie));
        }

        connect(j, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)));
        job = j;
    }

    void FeedRetriever::finished(KJob* j)
    {
        KIO::StoredTransferJob* stj = (KIO::StoredTransferJob*)j;
        err = stj->error();
        QByteArray data = stj->data();
        if (!err && !backup_file.isEmpty())
        {
            QFile fptr(backup_file);
            if (fptr.open(QIODevice::WriteOnly))
            {
                fptr.write(data);
                fptr.close();
            }
        }

        dataRetrieved(data, err == 0);
    }

}
