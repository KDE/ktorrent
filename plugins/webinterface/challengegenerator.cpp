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

#include <QXmlStreamWriter>

#include "httpserver.h"
#include "challengegenerator.h"
#include "httpresponseheader.h"
#include "httpclienthandler.h"

namespace kt
{

    ChallengeGenerator::ChallengeGenerator(HttpServer* server) : WebContentGenerator(server, "/login/challenge.xml", PUBLIC)
    {
    }


    ChallengeGenerator::~ChallengeGenerator()
    {
    }


    void ChallengeGenerator::get(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr)
    {
        Q_UNUSED(hdr);
        HttpResponseHeader rhdr(200);
        server->setDefaultResponseHeaders(rhdr, "text/xml", false);
        QByteArray output_data;
        QXmlStreamWriter out(&output_data);
        out.setAutoFormatting(true);
        out.writeStartDocument();
        out.writeStartElement("challenge");
        out.writeCharacters(server->challengeString());
        out.writeEndElement();
        out.writeEndDocument();
        hdlr->send(rhdr, output_data);
    }

    void ChallengeGenerator::post(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data)
    {
        Q_UNUSED(data);
        get(hdlr, hdr);
    }

}
