/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_TORRENTSERVICE_HH
#define KT_TORRENTSERVICE_HH

#include <DNSSD/PublicService>
#include <DNSSD/ServiceBrowser>

#include <interfaces/peersource.h>
#include <net/addressresolver.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
/**
 * Zeroconf service which publishes a torrent
 * */
class TorrentService : public bt::PeerSource
{
    Q_OBJECT
public:
    TorrentService(bt::TorrentInterface *tc);
    ~TorrentService() override;

    void stop(bt::WaitJob *wjob = nullptr) override;
    void start() override;
    void aboutToBeDestroyed() override;

Q_SIGNALS:
    void serviceDestroyed(TorrentService *av);

public Q_SLOTS:
    void onPublished(bool ok);
    void onServiceAdded(KDNSSD::RemoteService::Ptr ptr);
    void hostResolved(net::AddressResolver *ar);

private:
    bt::TorrentInterface *tc;
    KDNSSD::PublicService *srv;
    KDNSSD::ServiceBrowser *browser;
};
}

#endif
