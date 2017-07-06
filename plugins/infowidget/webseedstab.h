/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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

#ifndef KTWEBSEEDSTAB_H
#define KTWEBSEEDSTAB_H

#include <QSortFilterProxyModel>
#include <QWidget>

#include <KConfigGroup>
#include <KSharedConfig>

#include <interfaces/torrentinterface.h>
#include "ui_webseedstab.h"

namespace kt
{
    class WebSeedsModel;

    /**
        Tab which displays the list of webseeds of a torrent, and allows you to add or remove them.
    */
    class WebSeedsTab : public QWidget, public Ui_WebSeedsTab
    {
        Q_OBJECT
    public:
        WebSeedsTab(QWidget* parent);
        ~WebSeedsTab();

        /**
         * Switch to a different torrent.
         * @param tc The torrent
         */
        void changeTC(bt::TorrentInterface* tc);

        /// Check to see if the GUI needs to be updated
        void update();

        void saveState(KSharedConfigPtr cfg);
        void loadState(KSharedConfigPtr cfg);

    private slots:
        void addWebSeed();
        void removeWebSeed();
        void disableAll();
        void enableAll();
        void onWebSeedTextChanged(const QString& ws);
        void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    private:
        void selectionChanged(const QModelIndexList& indexes);

    private:
        bt::TorrentInterface::WPtr curr_tc;
        WebSeedsModel* model;
        QSortFilterProxyModel* proxy_model;
    };

}

#endif
