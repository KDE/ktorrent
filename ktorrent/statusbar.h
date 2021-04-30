/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_STATUSBAR_HH
#define KT_STATUSBAR_HH

#include <QStatusBar>
#include <interfaces/guiinterface.h>
#include <util/constants.h>

class QLabel;

namespace dht
{
struct Stats;
}

namespace kt
{
/**
 * Class which handles the statusbar
 * */
class StatusBar : public QStatusBar, public StatusBarInterface
{
    Q_OBJECT
public:
    StatusBar(QWidget *parent);
    ~StatusBar() override;

    /// Update the speed info of the status bar (speeds are in bytes per sec)
    void updateSpeed(bt::Uint32 up, bt::Uint32 down);

    /// Update the number of bytes transferred
    void updateTransfer(bt::Uint64 up, bt::Uint64 down);

    /// Update the DHT stats
    void updateDHTStatus(bool on, const dht::Stats &s);

    /// Create a progress bar and put it on the right side of the statusbar
    QProgressBar *createProgressBar() override;

    /// Remove a progress bar created with createProgressBar
    void removeProgressBar(QProgressBar *pb) override;
public Q_SLOTS:
    /// Show an information message
    void message(const QString &msg) override;

private:
    QLabel *speed;
    bt::Uint32 up_speed;
    bt::Uint32 down_speed;

    QLabel *transfer;
    bt::Uint64 up_transfer;
    bt::Uint64 down_transfer;

    QLabel *dht_status;
    bt::Uint32 dht_peers;
    bt::Uint32 dht_tasks;
    bool dht_on;
};
}

#endif
