/***************************************************************************
 *   Copyright (C) 2006-2007 by Joris Guisson, Ivan Vasic                  *
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

#ifndef TRACKERVIEW_H
#define TRACKERVIEW_H

#include "ui_trackerview.h"

#include <KSharedConfig>
#include <QSortFilterProxyModel>

#include <interfaces/torrentinterface.h>

namespace kt
{
    class TrackerModel;

    /**
     * @author Ivan Vasic <ivan@ktorrent.org>
     */
    class TrackerView: public QWidget, public Ui_TrackerView
    {
        Q_OBJECT
    public:
        TrackerView(QWidget* parent);
        ~TrackerView();

        void update();
        void changeTC(bt::TorrentInterface* ti);
        void saveState(KSharedConfigPtr cfg);
        void loadState(KSharedConfigPtr cfg);

    public slots:
        void updateClicked();
        void restoreClicked();
        void changeClicked();
        void removeClicked();
        void addClicked();
        void scrapeClicked();
        void currentChanged(const QModelIndex& current, const QModelIndex& previous);

    private:
        void torrentChanged(bt::TorrentInterface* ti);

    private:
        bt::TorrentInterface::WPtr tc;
        TrackerModel* model;
        QSortFilterProxyModel* proxy_model;
        QStringList tracker_hints;
        bool header_state_loaded;
    };
}
#endif
