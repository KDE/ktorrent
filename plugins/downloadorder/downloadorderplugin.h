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
#ifndef KTDOWNLOADORDERPLUGIN_H
#define KTDOWNLOADORDERPLUGIN_H

#include <util/ptrmap.h>
#include <interfaces/torrentactivityinterface.h>
#include <interfaces/plugin.h>

namespace kt
{
    class DownloadOrderManager;

    /**
        @author
    */
    class DownloadOrderPlugin : public Plugin, public ViewListener
    {
        Q_OBJECT
    public:
        DownloadOrderPlugin(QObject* parent, const QVariantList& args);
        ~DownloadOrderPlugin();

        bool versionCheck(const QString& version) const override;
        void load() override;
        void unload() override;
        void currentTorrentChanged(bt::TorrentInterface* tc) override;
        QString parentPart() const override {return QStringLiteral("torrentactivity");}

        /// Get the download order manager for a torrent (returns 0 if none exists)
        DownloadOrderManager* manager(bt::TorrentInterface* tc);

        /// Create a manager for a torrent
        DownloadOrderManager* createManager(bt::TorrentInterface* tc);

        /// Destroy a manager
        void destroyManager(bt::TorrentInterface* tc);

    private slots:
        void showDownloadOrderDialog();
        void torrentAdded(bt::TorrentInterface* tc);
        void torrentRemoved(bt::TorrentInterface* tc);

    private:
        QAction * download_order_action;
        bt::PtrMap<bt::TorrentInterface*, DownloadOrderManager> managers;
    };

}

#endif
