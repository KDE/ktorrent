/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "statusbar.h"

#include <KLocalizedString>
#include <QLabel>
#include <QProgressBar>

#include <dht/dhtbase.h>
#include <util/functions.h>

#include "statusbarofflineindicator.h"
using namespace bt;

namespace kt
{
StatusBar::StatusBar(QWidget *parent)
    : QStatusBar(parent)
    , speed(nullptr)
    , up_speed(0)
    , down_speed(0)
    , transfer(nullptr)
    , up_transfer(0)
    , down_transfer(0)
    , dht_status(nullptr)
    , dht_peers(0)
    , dht_tasks(0)
    , dht_on(false)
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

    addPermanentWidget(new StatusBarOfflineIndicator(this));
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

void StatusBar::updateDHTStatus(bool on, const dht::Stats &s)
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

QProgressBar *StatusBar::createProgressBar()
{
    QProgressBar *pb = new QProgressBar(this);
    pb->setMaximumHeight(height());
    addPermanentWidget(pb);
    return pb;
}

void StatusBar::removeProgressBar(QProgressBar *pb)
{
    removeWidget(pb);
    pb->deleteLater();
}

void StatusBar::message(const QString &msg)
{
    showMessage(msg, 30 * 1000);
}
}
