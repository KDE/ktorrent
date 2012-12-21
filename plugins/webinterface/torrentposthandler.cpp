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
#include <QFile>
#include <util/log.h>
#include <interfaces/functions.h>
#include <interfaces/coreinterface.h>
#include "torrentposthandler.h"
#include "httpresponseheader.h"
#include "httpclienthandler.h"
#include "httpserver.h"
#include <klocalizedstring.h>

using namespace bt;

namespace kt
{

    TorrentPostHandler::TorrentPostHandler(CoreInterface* core, HttpServer* server) : WebContentGenerator(server, "/torrent/load", LOGIN_REQUIRED), core(core)
    {
    }


    TorrentPostHandler::~TorrentPostHandler()
    {
    }


    void TorrentPostHandler::get(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr)
    {
        Q_UNUSED(hdr);
        // Send internal server error
        HttpResponseHeader rhdr(500);
        server->setDefaultResponseHeaders(rhdr, "text/html", false);
        hdlr->send500(rhdr, i18n("HTTP Get not supported when uploading a torrent"));
    }

    void TorrentPostHandler::post(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data)
    {
        const char* ptr = data.data();
        int len = data.size();
        int pos = QString(data).indexOf("\r\n\r\n");

        if (pos == -1 || pos + 4 >= len)
        {
            HttpResponseHeader rhdr(500);
            server->setDefaultResponseHeaders(rhdr, "text/html", false);
            hdlr->send500(rhdr, i18n("Invalid data received"));
            return;
        }

        // save torrent to a temporary file
        QString save_file = kt::DataDir() + "webgui_load_torrent";
        QFile tmp_file(save_file);

        if (!tmp_file.open(QIODevice::WriteOnly))
        {
            HttpResponseHeader rhdr(500);
            server->setDefaultResponseHeaders(rhdr, "text/html", false);
            hdlr->send500(rhdr, i18n("Failed to open temporary file"));
            return;
        }

        QDataStream out(&tmp_file);
        out.writeRawData(ptr + (pos + 4), len - (pos + 4));
        tmp_file.close();

        Out(SYS_WEB | LOG_NOTICE) << "Loading file " << save_file << endl;
        core->loadSilently(KUrl(save_file), QString());

        KUrl url;
        url.setEncodedPathAndQuery(hdr.path());
        QString page = url.queryItem("page");
        // there needs to be a page to send back
        if (page.isEmpty())
        {
            server->redirectToLoginPage(hdlr);
        }
        else
        {
            // redirect to page mentioned in page parameter
            HttpResponseHeader rhdr(301);
            server->setDefaultResponseHeaders(rhdr, "text/html", true);
            rhdr.setValue("Location", "/" + page);
            hdlr->send(rhdr, QByteArray());
        }
    }

}
