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
#include <kio/copyjob.h>
#include <util/fileops.h>
#include <util/log.h>
#include "opensearchdownloadjob.h"

using namespace bt;

namespace kt
{

    OpenSearchDownloadJob::OpenSearchDownloadJob(const QUrl &url, const QString& dir) : url(url), dir(dir)
    {
    }


    OpenSearchDownloadJob::~OpenSearchDownloadJob()
    {
    }

    void OpenSearchDownloadJob::start()
    {
        // first try to download the html page
        KIO::StoredTransferJob* j = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
        connect(j, SIGNAL(result(KJob*)), this, SLOT(getFinished(KJob*)));
    }

    void OpenSearchDownloadJob::getFinished(KJob* j)
    {
        if (j->error())
        {
            setError(j->error());
            emitResult();
            return;
        }

        QString str = QString(((KIO::StoredTransferJob*)j)->data());

        // try to find the link tags
        QRegExp rx(QLatin1String("<link([^<>]*)"), Qt::CaseInsensitive);
        int pos = 0;

        while ((pos = rx.indexIn(str, pos)) != -1)
        {
            QString link_tag = rx.cap(1);
            // exit when we find the description
            if (checkLinkTagContent(link_tag))
                return;

            pos += rx.matchedLength();
        }


        // no link tag found emit error
        setError(KIO::ERR_INTERNAL);
        emitResult();
    }

    bool OpenSearchDownloadJob::checkLinkTagContent(const QString& content)
    {
        if (htmlParam("type", content) != QLatin1String("application/opensearchdescription+xml"))
            return false;

        QString href = htmlParam("href", content);
        if (href.isEmpty())
            return false;

        if (href.startsWith('/'))
            href = url.scheme() + QLatin1String("://") + url.host() + href;

        if (!bt::Exists(dir))
        {
            try
            {
                bt::MakeDir(dir);
            }
            catch (...)
            {
                return false;
            }
        }

        // href is the opensearch description, so lets try to download it
        KIO::Job* j = KIO::copy(QUrl(href), QUrl::fromLocalFile(dir + QLatin1String("opensearch.xml")), KIO::HideProgressInfo);
        connect(j, SIGNAL(result(KJob*)), this, SLOT(xmlFileDownloadFinished(KJob*)));
        return true;
    }

    QString OpenSearchDownloadJob::htmlParam(const QString& param, const QString& content)
    {
        QRegExp rx(QString::fromLatin1("%1=\"?([^\">< ]*)[\" ]").arg(param), Qt::CaseInsensitive);
        if (rx.indexIn(content, 0) == -1)
            return QString();

        return rx.cap(1);
    }

    void OpenSearchDownloadJob::xmlFileDownloadFinished(KJob* j)
    {
        if (j->error())
        {
            setError(j->error());
            emitResult();
        }
        else
        {
            setError(0);
            emitResult();
        }
    }
}
