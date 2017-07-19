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

#include "torrentservice.h"

#include <cstdlib>
#include <util/log.h>
#include <util/sha1hash.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <peer/peerid.h>
#include <interfaces/torrentinterface.h>

using namespace bt;

namespace kt
{
    TorrentService::TorrentService(TorrentInterface* tc) : tc(tc), srv(nullptr), browser(nullptr)
    {
    }

    TorrentService::~TorrentService()
    {
        stop(nullptr);
    }

    void TorrentService::onPublished(bool ok)
    {
        if (ok)
            Out(SYS_ZCO | LOG_NOTICE) << "ZC: " << tc->getStats().torrent_name << " was published" << endl;
        else
            Out(SYS_ZCO | LOG_NOTICE) << "ZC: failed to publish " << tc->getStats().torrent_name  << endl;
    }

    void TorrentService::stop(bt::WaitJob* wjob)
    {
        Q_UNUSED(wjob);
        if (srv)
        {
            srv->stop();
            srv->deleteLater();
            srv = nullptr;
        }

        if (browser)
        {
            browser->deleteLater();
            browser = nullptr;
        }
    }

    void TorrentService::start()
    {
        bt::Uint16 port = bt::ServerInterface::getPort();
        QString name = QStringLiteral("%1__%2%3").arg(tc->getOwnPeerID().toString()).arg((rand() % 26) + 65).arg((rand() % 26) + 65);
        QStringList subtypes;
        subtypes << QLatin1Char('_') + tc->getInfoHash().toString() + QStringLiteral("._sub._bittorrent._tcp");

        if (!srv)
        {
            srv = new KDNSSD::PublicService();

            srv->setPort(port);
            srv->setServiceName(name);
            srv->setType(QStringLiteral("_bittorrent._tcp"));
            srv->setSubTypes(subtypes);

            connect(srv, &KDNSSD::PublicService::published, this, &TorrentService::onPublished);
            srv->publishAsync();
        }


        if (!browser)
        {
            browser = new KDNSSD::ServiceBrowser(QLatin1Char('_') + tc->getInfoHash().toString() + QStringLiteral("._sub._bittorrent._tcp"), true);
            connect(browser, &KDNSSD::ServiceBrowser::serviceAdded, this, &TorrentService::onServiceAdded);
            browser->startBrowse();
        }
    }

    void TorrentService::onServiceAdded(KDNSSD::RemoteService::Ptr ptr)
    {
        // lets not connect to ourselves
        if (!ptr->serviceName().startsWith(tc->getOwnPeerID().toString()))
        {
            QString host = ptr->hostName();
            bt::Uint16 port = ptr->port();
            Out(SYS_ZCO | LOG_NOTICE) << "ZC: found local peer " << host << ":" << port << endl;
            // resolve host name before adding it
            net::AddressResolver::resolve(host, port, this, SLOT(hostResolved(net::AddressResolver*)));
        }
    }

    void TorrentService::hostResolved(net::AddressResolver* ar)
    {
        if (ar->succeeded())
        {
            addPeer(ar->address(), true);
            peersReady(this);
        }
    }

    void TorrentService::aboutToBeDestroyed()
    {
        serviceDestroyed(this);
    }
}

