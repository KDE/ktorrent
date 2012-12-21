/***************************************************************************
*   Copyright (C) 2006 by Diego R. Brogna                                 *
*   dierbro@gmail.com                                                     *
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
#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <qcache.h>
#include <qhttp.h>
#include <qdatetime.h>
#include <net/serversocket.h>
#include <util/ptrmap.h>
#include "webcontentgenerator.h"

class QSocketNotifier;

namespace bt
{
    class MMapFile;
}

namespace kt
{
    class CoreInterface;

    /**
     * @author Diego R. Brogna
     */
    struct Session
    {
        bool logged_in;
        QTime last_access;
        int sessionId;
    };

    struct HeaderField
    {
        bool gzip;
        bool keepAlive;
        int sessionId;
        bool ifModifiedSince;
    };

    class HttpClientHandler;
    class HttpResponseHeader;



    class HttpServer : public QObject, public net::ServerSocket::ConnectionHandler
    {
        Q_OBJECT
    public:
        HttpServer(CoreInterface* core, bt::Uint16 port);
        virtual ~HttpServer();

        bool isOK() const {return sockets.count() > 0;}
        bt::Uint16 getPort() const {return port;}

        void handleGet(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr);
        void handlePost(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data);
        void handleUnsupportedMethod(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr);
        bt::MMapFile* cacheLookup(const QString& name);
        void insertIntoCache(const QString& name, bt::MMapFile* file);
        QString challengeString();
        void addContentGenerator(WebContentGenerator* g);
        void setDefaultResponseHeaders(HttpResponseHeader& hdr, const QString& content_type, bool with_session_info);
        bool checkLogin(const QHttpRequestHeader& hdr, const QByteArray& data);
        void redirectToLoginPage(HttpClientHandler* hdlr);
        void logout();
        void handleNormalFile(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QString& path);

    protected slots:
        void slotConnectionClosed();

    private:
        bool checkSession(const QHttpRequestHeader& hdr);
        QDateTime parseDate(const QString& str);
        QString skinDir() const;
        QString commonDir() const;
        void handleFile(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QString& path);
        virtual void newConnection(int fd, const net::Address& addr);

    private:
        QList<net::ServerSocket::Ptr> sockets;
        QString rootDir;
        int sessionTTL;
        Session session;
        CoreInterface* core;
        QCache<QString, bt::MMapFile> cache;
        bt::Uint16 port;
        QStringList skin_list;
        QString challenge;
        bt::PtrMap<QString, WebContentGenerator> content_generators;
        QList<HttpClientHandler*> clients;
    };


}
#endif // HTTPSERVER_H
