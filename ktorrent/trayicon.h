/***************************************************************************
 *   Copyright (C) 2005-2007 by                                            *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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

#ifndef TRAYICON_H
#define TRAYICON_H

#include <QMenu>
#include <KStatusNotifierItem>

#include <util/constants.h>
#include <interfaces/torrentinterface.h>

using namespace bt;

class QString;

namespace kt
{

    struct CurrentStats;

    class Core;

    class SetMaxRate;

    class GUI;

    struct TrayStats
    {
        bt::Uint32 download_speed;
        bt::Uint32 upload_speed;
        bt::Uint64 bytes_downloaded;
        bt::Uint64 bytes_uploaded;
    };

    /**
     * @author Joris Guisson
     * @author Ivan Vasic
    */

    class TrayIcon : public QObject
    {
        Q_OBJECT
    public:
        TrayIcon(Core* tc, GUI* parent);
        ~TrayIcon();

        /// Update stats for system tray icon
        void updateStats(const CurrentStats& stats);

        /// Update the max rate menus
        void updateMaxRateMenus();

        /// Show the icon
        void show();

        /// Hide the icon
        void hide();

        /// Get the co
        QMenu* contextMenu();

    private:
        void showPassivePopup(const QString& msg, const QString& titile);

    private slots:
        /**
                 * Show a passive popup, that a torrent has been silently added.
                 * @param tc The torrent
                 */
        void torrentSilentlyOpened(bt::TorrentInterface* tc);

        /**
         * Show a passive popup, that the torrent has stopped downloading.
         * @param tc The torrent
         */
        void finished(bt::TorrentInterface* tc);

        /**
         * Show a passive popup that a torrent has reached it's max share ratio.
         * @param tc The torrent
         */
        void maxShareRatioReached(bt::TorrentInterface* tc);

        /**
         * Show a passive popup that a torrent has reached it's max seed time
         * @param tc The torrent
         */
        void maxSeedTimeReached(bt::TorrentInterface* tc);

        /**
         * Show a passive popup when a torrent has been stopped by an error.
         * @param tc The torrent
         * @param msg Error message
         */
        void torrentStoppedByError(bt::TorrentInterface* tc, QString msg);

        /**
         * Corrupted data has been found.
         * @param tc The torrent
         */
        void corruptedData(bt::TorrentInterface* tc);

        /**
         * User tried to enqueue a torrent that has reached max share ratio or max seed time
         * Show passive popup message.
         */
        void queuingNotPossible(bt::TorrentInterface* tc);

        /**
         * We failed to start a torrent
         * @param tc The torrent
         * @param reason The reason it failed
         */
        void canNotStart(bt::TorrentInterface* tc, bt::TorrentStartResponse reason);

        ///Shows passive popup message
        void lowDiskSpace(bt::TorrentInterface* tc, bool stopped);

        /**
         * A torrent could not be loaded silently.
         * @param msg Message to show
         */
        void cannotLoadTorrentSilently(const QString& msg);

        /**
            The QM changes suspended state.
        */
        void suspendStateChanged(bool suspended);

        /**
            Show a warning message
        */
        void dhtNotEnabled(const QString& msg);

    private slots:
        void secondaryActivate(const QPoint& pos);

    private:
        Core* core;
        GUI* mwnd;
        int previousDownloadHeight;
        int previousUploadHeight;
        SetMaxRate* max_upload_rate;
        SetMaxRate* max_download_rate;
        KStatusNotifierItem* status_notifier_item;
        bool queue_suspended;
        QMenu* menu;
    };

    class SetMaxRate : public QMenu
    {
        Q_OBJECT
    public:
        enum Type
        {
            UPLOAD, DOWNLOAD
        };
        SetMaxRate(Core* tc, Type t, QWidget* parent);
        ~SetMaxRate();

    public slots:
        void update();

    private:
        void makeMenu();

    private slots:
        void onTriggered(QAction* act);

    private:
        Core* m_core;
        Type type;
        QAction* unlimited;
    };
}

#endif
