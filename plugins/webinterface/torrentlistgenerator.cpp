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

#include <util/sha1hash.h>
#include <util/functions.h>
#include <torrent/queuemanager.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include "httpserver.h"
#include "torrentlistgenerator.h"
#include "httpresponseheader.h"
#include "httpclienthandler.h"

using namespace bt;

namespace kt
{

    TorrentListGenerator::TorrentListGenerator(CoreInterface* core, HttpServer* server)
        : WebContentGenerator(server, "/data/torrents.xml", LOGIN_REQUIRED), core(core)
    {
    }


    TorrentListGenerator::~TorrentListGenerator()
    {
    }


    void TorrentListGenerator::get(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr)
    {
        Q_UNUSED(hdr);
        HttpResponseHeader rhdr(200);
        server->setDefaultResponseHeaders(rhdr, "text/xml", true);

        QByteArray output_data;
        QXmlStreamWriter out(&output_data);
        out.setAutoFormatting(true);
        out.writeStartDocument();
        out.writeStartElement("torrents");
        kt::QueueManager* qman = core->getQueueManager();
        kt::QueueManager::iterator i = qman->begin();
        while (i != qman->end())
        {
            bt::TorrentInterface* ti = *i;
            const bt::TorrentStats& s = ti->getStats();
            out.writeStartElement("torrent");
            writeElement(out, "name", ti->getDisplayName());
            writeElement(out, "info_hash", ti->getInfoHash().toString());
            writeElement(out, "status", s.statusToString());
            writeElement(out, "bytes_downloaded", BytesToString(s.bytes_downloaded));
            writeElement(out, "bytes_uploaded", BytesToString(s.bytes_uploaded));
            writeElement(out, "total_bytes", BytesToString(s.total_bytes));
            writeElement(out, "total_bytes_to_download", BytesToString(s.total_bytes_to_download));
            writeElement(out, "download_rate", BytesPerSecToString(s.download_rate));
            writeElement(out, "upload_rate", BytesPerSecToString(s.upload_rate));
            writeElement(out, "num_peers", QString::number(s.num_peers));
            writeElement(out, "seeders", QString::number(s.seeders_connected_to));
            writeElement(out, "seeders_total", QString::number(s.seeders_total));
            writeElement(out, "leechers", QString::number(s.leechers_connected_to));
            writeElement(out, "leechers_total", QString::number(s.leechers_total));
            writeElement(out, "running", s.running ? "1" : "0");
            writeElement(out, "percentage", QString::number(Percentage(s), 'f', 2));
            writeElement(out, "num_files", QString::number(ti->getNumFiles()));
            out.writeEndElement();
            i++;
        }
        out.writeEndElement();
        out.writeEndDocument();
        hdlr->send(rhdr, output_data);
    }

    void TorrentListGenerator::writeElement(QXmlStreamWriter& out, const QString& name, const QString& value)
    {
        out.writeStartElement(name);
        out.writeCharacters(value);
        out.writeEndElement();
    }

    void TorrentListGenerator::post(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data)
    {
        Q_UNUSED(data);
        get(hdlr, hdr);
    }
}
