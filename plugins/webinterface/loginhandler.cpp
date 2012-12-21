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
#include <kurl.h>
#include "loginhandler.h"
#include "httpserver.h"
#include "httpresponseheader.h"
#include "httpclienthandler.h"
#include "webinterfacepluginsettings.h"

namespace kt
{

    LoginHandler::LoginHandler(HttpServer* server): WebContentGenerator(server, "/login", PUBLIC)
    {
    }


    LoginHandler::~LoginHandler()
    {
    }


    void LoginHandler::get(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr)
    {
        Q_UNUSED(hdr);
        // we shouldn't get a get request, so redirect to login page
        server->redirectToLoginPage(hdlr);
    }

    void LoginHandler::post(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data)
    {
        KUrl url;
        url.setEncodedPathAndQuery(hdr.path());

        QString page = url.queryItem("page");
        // there needs to be a page to send back
        if (page.isEmpty() && WebInterfacePluginSettings::authentication())
        {
            server->redirectToLoginPage(hdlr);
            return;
        }

        if (server->checkLogin(hdr, data))
        {
            // login is OK, so redirect to page
            HttpResponseHeader rhdr(301);
            server->setDefaultResponseHeaders(rhdr, "text/html", true);
            rhdr.setValue("Location", "/" + page);
            hdlr->send(rhdr, QByteArray());
        }
        else
        {
            // login failed
            server->redirectToLoginPage(hdlr);
        }
    }

}
