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

#ifndef KT_STATUSBAR_HH
#define KT_STATUSBAR_HH

#include <QStatusBar>
#include <util/constants.h>
#include <interfaces/guiinterface.h>

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
        StatusBar(QWidget* parent);
        ~StatusBar();

        /// Update the speed info of the status bar (speeds are in bytes per sec)
        void updateSpeed(bt::Uint32 up, bt::Uint32 down);

        /// Update the number of bytes tranfered
        void updateTransfer(bt::Uint64 up, bt::Uint64 down);

        /// Update the DHT stats
        void updateDHTStatus(bool on, const dht::Stats& s);

        /// Create a progress bar and put it on the right side of the statusbar
        QProgressBar* createProgressBar() override;

        /// Remove a progress bar created with createProgressBar
        void removeProgressBar(QProgressBar* pb) override;
    public slots:
        /// Show an information message
        void message(const QString& msg) override;

    private:
        QLabel* speed;
        bt::Uint32 up_speed;
        bt::Uint32 down_speed;

        QLabel* transfer;
        bt::Uint64 up_transfer;
        bt::Uint64 down_transfer;

        QLabel* dht_status;
        bt::Uint32 dht_peers;
        bt::Uint32 dht_tasks;
        bool dht_on;
    };
}

#endif
