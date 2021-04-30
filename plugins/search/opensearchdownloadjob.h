/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTOPENSEARCHDOWNLOADJOB_H
#define KTOPENSEARCHDOWNLOADJOB_H

#include <KIO/Job>
#include <QUrl>

#include "proxy_helper.h"

namespace kt
{
/**
    Job which tries to find an opensearch xml description on a website and
    download that to a directory.
*/
class OpenSearchDownloadJob : public KIO::Job
{
    Q_OBJECT
public:
    OpenSearchDownloadJob(const QUrl &url, const QString &dir, ProxyHelper *proxy);
    ~OpenSearchDownloadJob() override;

    /// Start the job
    void start() override;

    /// Start the job. Try to get file by default url
    void startDefault();

    /// Get the directory
    QString directory() const
    {
        return dir;
    }

    /// Get the hostname
    QString hostname() const
    {
        return url.host();
    }

private Q_SLOTS:
    void getFinished(KJob *j);
    void xmlFileDownloadFinished(KJob *j);

private:
    bool checkLinkTagContent(const QString &content);
    QString htmlParam(const QString &param, const QString &content);
    bool startXMLDownload(const QUrl &url);

private:
    QUrl url;
    QString dir;
    ProxyHelper *m_proxy;
};

}

#endif
