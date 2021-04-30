/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "torrentservice.h"

#include <QRandomGenerator>

#include <interfaces/torrentinterface.h>
#include <peer/peerid.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <util/log.h>
#include <util/sha1hash.h>

using namespace bt;

namespace kt
{
TorrentService::TorrentService(TorrentInterface *tc)
    : tc(tc)
    , srv(nullptr)
    , browser(nullptr)
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
        Out(SYS_ZCO | LOG_NOTICE) << "ZC: failed to publish " << tc->getStats().torrent_name << endl;
}

void TorrentService::stop(bt::WaitJob *wjob)
{
    Q_UNUSED(wjob)
    if (srv) {
        srv->stop();
        srv->deleteLater();
        srv = nullptr;
    }

    if (browser) {
        browser->deleteLater();
        browser = nullptr;
    }
}

void TorrentService::start()
{
    bt::Uint16 port = bt::ServerInterface::getPort();
    QString name = QStringLiteral("%1__%2%3")
                       .arg(tc->getOwnPeerID().toString())
                       .arg(QRandomGenerator::global()->bounded(26) + 65)
                       .arg(QRandomGenerator::global()->bounded(26) + 65);
    QStringList subtypes;
    subtypes << QLatin1Char('_') + tc->getInfoHash().toString() + QStringLiteral("._sub._bittorrent._tcp");

    if (!srv) {
        srv = new KDNSSD::PublicService();

        srv->setPort(port);
        srv->setServiceName(name);
        srv->setType(QStringLiteral("_bittorrent._tcp"));
        srv->setSubTypes(subtypes);

        connect(srv, &KDNSSD::PublicService::published, this, &TorrentService::onPublished);
        srv->publishAsync();
    }

    if (!browser) {
        browser = new KDNSSD::ServiceBrowser(QLatin1Char('_') + tc->getInfoHash().toString() + QStringLiteral("._sub._bittorrent._tcp"), true);
        connect(browser, &KDNSSD::ServiceBrowser::serviceAdded, this, &TorrentService::onServiceAdded);
        browser->startBrowse();
    }
}

void TorrentService::onServiceAdded(KDNSSD::RemoteService::Ptr ptr)
{
    // lets not connect to ourselves
    if (!ptr->serviceName().startsWith(tc->getOwnPeerID().toString())) {
        QString host = ptr->hostName();
        bt::Uint16 port = ptr->port();
        Out(SYS_ZCO | LOG_NOTICE) << "ZC: found local peer " << host << ":" << port << endl;
        // resolve host name before adding it
        // clang-format off
        net::AddressResolver::resolve(host, port, this, SLOT(hostResolved(net::AddressResolver*)));
        // clang-format on
    }
}

void TorrentService::hostResolved(net::AddressResolver *ar)
{
    if (ar->succeeded()) {
        addPeer(ar->address(), true);
        peersReady(this);
    }
}

void TorrentService::aboutToBeDestroyed()
{
    serviceDestroyed(this);
}
}
