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

#include <settings.h>
#include <util/sha1hash.h>
#include <util/functions.h>
#include <interfaces/coreinterface.h>
#include "httpserver.h"
#include "httpresponseheader.h"
#include "httpclienthandler.h"
#include "globaldatagenerator.h"

using namespace bt;

namespace kt
{

    GlobalDataGenerator::GlobalDataGenerator(CoreInterface* core, HttpServer* server)
        : WebContentGenerator(server, "/data/global.xml", LOGIN_REQUIRED), core(core)
    {
    }


    GlobalDataGenerator::~GlobalDataGenerator()
    {
    }


    void GlobalDataGenerator::get(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr)
    {
        Q_UNUSED(hdr);
        HttpResponseHeader rhdr(200);
        server->setDefaultResponseHeaders(rhdr, "text/xml", true);

        CurrentStats s = core->getStats();
        QByteArray output_data;
        QXmlStreamWriter out(&output_data);
        out.setAutoFormatting(true);
        out.writeStartDocument();
        out.writeStartElement("global_data");
        writeElement(out, "transferred_down", BytesToString(s.bytes_downloaded));
        writeElement(out, "transferred_up", BytesToString(s.bytes_uploaded));
        writeElement(out, "speed_down", BytesPerSecToString(s.download_speed));
        writeElement(out, "speed_up", BytesPerSecToString(s.upload_speed));
        writeElement(out, "dht", Settings::dhtSupport() ? "1" : "0");
        writeElement(out, "encryption", Settings::useEncryption() ? "1" : "0");
        out.writeEndElement();
        out.writeEndDocument();
        hdlr->send(rhdr, output_data);
    }

    void GlobalDataGenerator::post(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data)
    {
        Q_UNUSED(data);
        get(hdlr, hdr);
    }

    void GlobalDataGenerator::writeElement(QXmlStreamWriter& out, const QString& name, const QString& value)
    {
        out.writeStartElement(name);
        out.writeCharacters(value);
        out.writeEndElement();
    }
}
