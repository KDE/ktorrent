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
        OpenSearchDownloadJob(const QUrl &url, const QString& dir, ProxyHelper *proxy);
        ~OpenSearchDownloadJob();

        /// Start the job
        void start();

        /// Start the job. Try to get file by default url
        void startDefault();

        /// Get the directory
        QString directory() const {return dir;}

        /// Get the hostname
        QString hostname() const {return url.host();}

    private slots:
        void getFinished(KJob* j);
        void xmlFileDownloadFinished(KJob* j);

    private:
        bool checkLinkTagContent(const QString& content);
        QString htmlParam(const QString& param, const QString& content);
        bool startXMLDownload(const QUrl& url);

    private:
        QUrl url;
        QString dir;
        ProxyHelper* m_proxy;
    };

}

#endif
