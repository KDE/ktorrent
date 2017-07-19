/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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

#include <KGuiItem>
#include <KStandardGuiItem>

#include <interfaces/torrentinterface.h>
#include "addpeersdlg.h"

namespace kt
{
    class ManualPeerSource : public bt::PeerSource
    {
    public:
        ManualPeerSource() {}
        ~ManualPeerSource() {}

        void start() override {}
        void stop (bt::WaitJob*) override {}

        void add(const QString& ip, bt::Uint16 port)
        {
            addPeer(net::Address(ip, port), false);
            peersReady(this);
        }
    };

    AddPeersDlg::AddPeersDlg(bt::TorrentInterface* tc, QWidget* parent)
        : QDialog(parent), tc(tc), mps(nullptr)
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

