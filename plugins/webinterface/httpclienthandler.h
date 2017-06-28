/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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

#ifndef KTHTTPCLIENTHANDLER_H
#define KTHTTPCLIENTHANDLER_H

#include <qhttp.h>
#include <net/socket.h>
#include <util/constants.h>
#include "httpresponseheader.h"

class QSocketNotifier;

namespace kt
{
    class HttpServer;

    /**
        @author Joris Guisson <joris.guisson@gmail.com>
    */
    class HttpClientHandler : public QObject
    {
        Q_OBJECT
        enum State
        {
            WAITING_FOR_REQUEST,
            WAITING_FOR_CONTENT
        };
    public:
        HttpClientHandler(HttpServer* srv, int sock);
        virtual ~HttpClientHandler();


        bool sendFile(HttpResponseHeader& hdr, const QString& full_path);
        void sendResponse(HttpResponseHeader& hdr);
        void send404(HttpResponseHeader& hdr, const QString& path);
        void send500(HttpResponseHeader& hdr, const QString& error);
        void send(HttpResponseHeader& hdr, const QByteArray& data);
        bool shouldClose() const;

    private:
        void handleRequest(int header_len);
        void setResponseHeaders(HttpResponseHeader& hdr);

    private slots:
        void readyToRead(int);
        void sendOutputBuffer(int fd = 0);

    signals:
        void closed();

    private:
        HttpServer* srv;
        net::Socket* client;
        QSocketNotifier* read_notifier;
        QSocketNotifier* write_notifier;
        State state;
        QHttpRequestHeader header;
        QByteArray data;
        bt::Uint32 bytes_read;
        HttpResponseHeader php_response_hdr;
        QByteArray output_buffer;
        bt::Uint32 written;
    };

}

#endif
