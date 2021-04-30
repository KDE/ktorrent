/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KIO/Job>

#include "feedretriever.h"
#include <ktversion.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
FeedRetriever::FeedRetriever()
    : job(nullptr)
    , err(0)
{
}

FeedRetriever::FeedRetriever(const QString &file_name)
    : backup_file(file_name)
    , job(nullptr)
    , err(0)
{
}

FeedRetriever::~FeedRetriever()
{
}

void FeedRetriever::setAuthenticationCookie(const QString &cookie)
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

void FeedRetriever::retrieveData(const QUrl &url)
{
    KIO::StoredTransferJob *j = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    j->addMetaData(QStringLiteral("UserAgent"), bt::GetVersionString());
    if (!cookie.isEmpty()) {
        j->addMetaData(QStringLiteral("cookies"), QStringLiteral("none"));
        j->addMetaData(QStringLiteral("customHTTPHeader"), QStringLiteral("Cookie: %1").arg(cookie));
    }

    connect(j, &KIO::StoredTransferJob::result, this, &FeedRetriever::finished);
    job = j;
}

void FeedRetriever::finished(KJob *j)
{
    KIO::StoredTransferJob *stj = (KIO::StoredTransferJob *)j;
    err = stj->error();
    QByteArray data = stj->data();
    if (!err && !backup_file.isEmpty()) {
        QFile fptr(backup_file);
        if (fptr.open(QIODevice::WriteOnly)) {
            fptr.write(data);
            fptr.close();
        }
    }

    dataRetrieved(data, err == 0);
}

}
