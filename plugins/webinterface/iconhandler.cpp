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
#include <kglobal.h>
#include <kiconloader.h>
#include "iconhandler.h"
#include "httpclienthandler.h"
#include "httpserver.h"

namespace kt
{

    IconHandler::IconHandler(HttpServer* server) : WebContentGenerator(server, "/icon", PUBLIC)
    {
    }


    IconHandler::~IconHandler()
    {
    }


    void IconHandler::get(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr)
    {
        KUrl url;
        url.setEncodedPathAndQuery(hdr.path());
        QString name = url.queryItem("name");

        int size = url.queryItem("size").toInt();
        if (size < KIconLoader::NoGroup)
            size = KIconLoader::NoGroup;
        else if (size > KIconLoader::User)
            size = KIconLoader::User;

        server->handleNormalFile(hdlr, hdr, KIconLoader::global()->iconPath(name, size));
    }

    void IconHandler::post(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data)
    {
        Q_UNUSED(data);
        get(hdlr, hdr);
    }

}
