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

#include "statusbar.h"

#include <QLabel>
#include <QProgressBar>
#include <KLocalizedString>
#include <KStatusBarOfflineIndicator>

#include <util/functions.h>
#include <dht/dhtbase.h>

using namespace bt;

namespace kt
{
    StatusBar::StatusBar(QWidget* parent)
        : QStatusBar(parent),
          speed(nullptr), up_speed(0), down_speed(0),
          transfer(nullptr), up_transfer(0), down_transfer(0),
          dht_status(nullptr), dht_peers(0), dht_tasks(0), dht_on(false)
    {
        QString s = i18n("Speed down: %1 / up: %2", BytesPerSecToString((double)down_speed), BytesPerSecToString((double)up_speed));
        QString t = i18n("Transferred down: %1 / up: %2", BytesToString(down_transfer), BytesToString(up_transfer));

        dht_status = new QLabel(i18n("DHT: off"), this);
        dht_status->setFrameShape(QFrame::Box);
        dht_status->setFrameShadow(QFrame::Sunken);
        addPermanentWidget(dht_status);

        speed = new QLabel(s, this);
        speed->setFrameShape(QFrame::Box);
        speed->setFrameShadow(QFrame::Sunken);
        addPermanentWidget(speed);

        transfer = new QLabel(t, this);
        transfer->setFrameShape(QFrame::Box);
        transfer->setFrameShadow(QFrame::Sunken);
        addPermanentWidget(transfer);

        addPermanentWidget(new KStatusBarOfflineIndicator(this));
    }

    StatusBar::~StatusBar()
    {
    }

    void StatusBar::updateSpeed(bt::Uint32 up, bt::Uint32 down)
    {
        if (up == up_speed && down == down_speed)
            return;

        up_speed = up;
        down_speed = down;
        QString s = i18n("Speed down: %1 / up: %2", BytesPerSecToString((double)down_speed), BytesPerSecToString((double)up_speed));
        speed->setText(s);
    }

    void StatusBar::updateTransfer(bt::Uint64 up, bt::Uint64 down)
    {
        if (up == up_transfer && down == down_transfer)
            return;

        up_transfer = up;
        down_transfer = down;
        QString t = i18n("Transferred down: %1 / up: %2", BytesToString(down_transfer), BytesToString(up_transfer));
        transfer->setText(t);
    }

    void StatusBar::updateDHTStatus(bool on, const dht::Stats& s)
    {
        if (on == dht_on && dht_peers == s.num_peers && dht_tasks == s.num_tasks)
            return;

        dht_on = on;
        dht_peers = s.num_peers;
        dht_tasks = s.num_tasks;
        if (on)
            dht_status->setText(i18n("DHT: %1, %2", i18np("%1 node", "%1 nodes", s.num_peers), i18np("%1 task", "%1 tasks", s.num_tasks)));
        else
            dht_status->setText(i18n("DHT: off"));
    }

    QProgressBar* StatusBar::createProgressBar()
    {
        QProgressBar* pb = new QProgressBar(this);
        pb->setMaximumHeight(height());
        addPermanentWidget(pb);
        return pb;
    }

    void StatusBar::removeProgressBar(QProgressBar* pb)
    {
        removeWidget(pb);
        pb->deleteLater();
    }

    void StatusBar::message(const QString& msg)
    {
        showMessage(msg, 30 * 1000);
    }
}
