/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "torrentactivityinterface.h"

namespace kt
{
TorrentActivityInterface::TorrentActivityInterface(const QString &name, const QString &icon, QWidget *parent)
    : Activity(name, icon, 0, parent)
{
}

TorrentActivityInterface::~TorrentActivityInterface()
{
}

void TorrentActivityInterface::notifyViewListeners(bt::TorrentInterface *tc)
{
    for (ViewListener *vl : qAsConst(listeners))
        vl->currentTorrentChanged(tc);
}

void TorrentActivityInterface::addViewListener(ViewListener *vl)
{
    listeners.append(vl);
}

void TorrentActivityInterface::removeViewListener(ViewListener *vl)
{
    listeners.removeAll(vl);
}
}
