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

#include <QSocketNotifier>
#include <qhttp.h>

#include <util/log.h>
#include <util/mmapfile.h>
#include <klocalizedstring.h>
#include "httpserver.h"
#include "httpclienthandler.h"
#include "httpresponseheader.h"


using namespace bt;

namespace kt
{

    HttpClientHandler::HttpClientHandler(HttpServer* srv, int sock) : srv(srv), client(nullptr), read_notifier(nullptr), write_notifier(nullptr), php_response_hdr(200)
    {
        client = new net::Socket(sock, 4);
        client->setBlocking(false);
        read_notifier = new QSocketNotifier(sock, QSocketNotifier::Read, this);
        connect(read_notifier, &QSocketNotifier::activated, this, &HttpClientHandler::readyToRead);
        write_notifier = new QSocketNotifier(sock, QSocketNotifier::Write, this);
        connect(write_notifier, SIGNAL(activated(int)), this, SLOT(sendOutputBuffer(int)));
        write_notifier->setEnabled(false);
        state = WAITING_FOR_REQUEST;
        bytes_read = 0;
        data.reserve(1024);
        output_buffer.reserve(4096);
        written = 0;
    }


    HttpClientHandler::~HttpClientHandler()
    {
        delete client;
    }

    void HttpClientHandler::readyToRead(int)
    {
        Uint32 ba = client->bytesAvailable();
        if (ba == 0)
        {
            // other side has closed connection
            client->close();
            closed();
            return;
        }

        if (state == WAITING_FOR_REQUEST)
        {
            Uint32 off = data.size();
            data.resize(data.size() + ba);
            client->recv((Uint8*)data.data() + off, ba);

            int end_of_req = data.indexOf("\r\n\r\n");
            if (end_of_req > 0)
            {
                // We have got the header, so lets parse it
                handleRequest(end_of_req + 4);
            }
        }
        else if (state == WAITING_FOR_CONTENT)
        {
            Uint32 ba = client->bytesAvailable();
            if (ba + bytes_read < header.contentLength())
            {
                Uint32 off = data.size();
                data.resize(off + ba);
                client->recv((Uint8*)data.data() + off, ba);
                bytes_read += ba;
            }
            else
            {
                Uint32 left = header.contentLength() - bytes_read;
                Uint32 off = data.size();
                data.resize(off + left);
                client->recv((Uint8*)data.data() + off, left);
                bytes_read += left;
                srv->handlePost(this, header, data);

                data.resize(0);
                state = WAITING_FOR_REQUEST;
                if (client->bytesAvailable() > 0)
                    readyToRead(client->fd());
            }
        }
    }

    void HttpClientHandler::handleRequest(int header_len)
    {
        header = QHttpRequestHeader(data.left(header_len)); // get the header
        data = data.mid(header_len); // move up the data
        //  Out(SYS_WEB|LOG_DEBUG) << "Parsing request : " << header.toString() << endl;
        if (header.method() == "POST")
        {
            if (header.hasContentLength())
            {
                bytes_read = data.size();
                if (bytes_read >= header.contentLength())
                {
                    srv->handlePost(this, header, data.left(header.contentLength()));
                    data = data.mid(header.contentLength()); // move past content
                }
                else
                    state = WAITING_FOR_CONTENT;
            }
        }
        else if (header.method() == "GET")
        {
            srv->handleGet(this, header);
        }
        else
        {
            srv->handleUnsupportedMethod(this, header);
        }

        if (client->bytesAvailable() > 0)
        {
            readyToRead(0);
        }
        else if (data.size() > 0 && state == WAITING_FOR_REQUEST)
        {
            // check if there is another request, if so handle in
            int eoh = data.indexOf("\r\n\r\n");
            if (eoh > 0)
                handleRequest(eoh + 4);
        }
    }

    bool HttpClientHandler::sendFile(HttpResponseHeader& hdr, const QString& full_path)
    {
        //  Out(SYS_WEB|LOG_DEBUG) << "Sending file " << full_path << endl;
        setResponseHeaders(hdr);
        // first look in cache
        MMapFile* c = srv->cacheLookup(full_path);

        if (!c)
        {
            // not in cache so load it
            c = new MMapFile();
            if (!c->open(full_path, QIODevice::ReadOnly))
            {
                delete c;
                Out(SYS_WEB | LOG_DEBUG) << "Failed to open file " << full_path << endl;
                return false;
            }
            srv->insertIntoCache(full_path, c);
        }

        //  Out(SYS_WEB|LOG_DEBUG) << "HTTP header : " << endl;
        //  Out(SYS_WEB|LOG_DEBUG) << hdr.toString() << endl;

        QByteArray data((const char*)c->getDataPointer(), c->getSize());
        hdr.setValue("Content-Length", QString::number(data.size()));
        output_buffer.append(hdr.toString().toUtf8());
        output_buffer.append(data);

        sendOutputBuffer();
        //  Out(SYS_WEB|LOG_DEBUG) << "Finished sending " << full_path << " (" << written << " bytes)" << endl;
        return true;
    }

#define HTTP_404_ERROR "<html><head><title>404 Not Found</title></head><body>The requested file %1 was not found !</body></html>"
#define HTTP_500_ERROR "<html><head><title>500 Internal Server Error</title></head><body><h1>Internal Server Error</h1><p>%1</p></body></html>"


    void HttpClientHandler::send404(HttpResponseHeader& hdr, const QString& path)
    {
        setResponseHeaders(hdr);
        //  Out(SYS_WEB|LOG_DEBUG) << "Sending 404 " << path << endl;
        QString data = QString(HTTP_404_ERROR).arg(path);
        hdr.setValue("Content-Length", QString::number(data.length()));

        output_buffer.append(hdr.toString().toUtf8());
        output_buffer.append(data.toUtf8());
        sendOutputBuffer();
    }

    void HttpClientHandler::send500(HttpResponseHeader& hdr, const QString& error)
    {
        setResponseHeaders(hdr);
        //  Out(SYS_WEB|LOG_DEBUG) << "Sending 500 " << endl;
        QString err = i18n("An internal server error occurred: %1", error);
        QString data = QString(HTTP_500_ERROR).arg(err);
        hdr.setValue("Content-Length", QString::number(data.length()));

        output_buffer.append(hdr.toString().toUtf8());
        output_buffer.append(data.toUtf8());
        sendOutputBuffer();
    }

    void HttpClientHandler::sendResponse(HttpResponseHeader& hdr)
    {
        setResponseHeaders(hdr);
        //  Out(SYS_WEB|LOG_DEBUG) << "Sending response " << hdr.toString() << endl;
        output_buffer.append(hdr.toString().toUtf8());
        sendOutputBuffer();
    }

    void HttpClientHandler::send(HttpResponseHeader& hdr, const QByteArray& data)
    {
        setResponseHeaders(hdr);
        hdr.setValue("Content-Length", QString::number(data.length()));
        output_buffer.append(hdr.toString().toUtf8());
        output_buffer.append(data);
        sendOutputBuffer();
    }

    void HttpClientHandler::sendOutputBuffer(int)
    {
        int r = client->send((const Uint8*)output_buffer.data() + written, output_buffer.size() - written);
        //Out(SYS_WEB|LOG_DEBUG) << "sendOutputBuffer : " << r << " " << written << " " << output_buffer.size() << endl;
        if (r <= 0)
        {
            // error happened, close the connection
            closed();
        }
        else
        {
            written += r;
            if (written == (Uint32)output_buffer.size())
            {
                // everything sent
                output_buffer.resize(0);
                write_notifier->setEnabled(false);
                written = 0;
                if (shouldClose())
                {
                    Out(SYS_WEB | LOG_DEBUG) << "closing HttpClientHandler" << endl;
                    client->close();
                    closed();
                }
            }
            else
            {
                // enable write_notifier, so we can send the rest later
                write_notifier->setEnabled(true);
            }
        }
    }

    bool HttpClientHandler::shouldClose() const
    {
        if (!header.isValid())
            return false;

        if (header.majorVersion() == 1 && header.minorVersion() == 0)
        {
            if (header.hasKey("Connection") && header.value("Connection").toLower() == "keep-alive")
                return false;
            else
                return true;
        }
        else
        {
            return header.hasKey("Connection") && header.value("Connection").toLower() == "close";
        }

        return false;
    }

    void HttpClientHandler::setResponseHeaders(HttpResponseHeader& hdr)
    {
        if (shouldClose())
        {
            if (header.majorVersion() == 1 && header.minorVersion() == 0)
                return;
            else
                hdr.setValue("Connection", "close");
        }
        else
        {
            if (header.majorVersion() == 1 && header.minorVersion() == 0)
                hdr.setValue("Connection", "Keep-Alive");
            else
                return;
        }
    }

}

#include "httpclienthandler.moc"

