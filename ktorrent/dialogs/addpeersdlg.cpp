/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KGuiItem>
#include <KStandardGuiItem>

#include "addpeersdlg.h"
#include <interfaces/torrentinterface.h>

namespace kt
{
class ManualPeerSource : public bt::PeerSource
{
public:
    ManualPeerSource()
    {
    }
    ~ManualPeerSource() override
    {
    }

    void start() override
    {
    }
    void stop(bt::WaitJob *) override
    {
    }

    void add(const QString &ip, bt::Uint16 port)
    {
        addPeer(net::Address(ip, port), false);
        peersReady(this);
    }
};

AddPeersDlg::AddPeersDlg(bt::TorrentInterface *tc, QWidget *parent)
    : QDialog(parent)
    , tc(tc)
    , mps(nullptr)
{
    setupUi(this);
    connect(m_close, &QPushButton::clicked, this, &AddPeersDlg::reject);
    connect(m_add, &QPushButton::clicked, this, &AddPeersDlg::addPressed);

    KGuiItem::assign(m_close, KStandardGuiItem::close());
    KGuiItem::assign(m_add, KStandardGuiItem::add());

    mps = new ManualPeerSource();
    tc->addPeerSource(mps);
}

AddPeersDlg::~AddPeersDlg()
{
    tc->removePeerSource(mps);
    delete mps;
}

void AddPeersDlg::addPressed()
{
    mps->add(m_ip->text(), m_port->value());
}

}
