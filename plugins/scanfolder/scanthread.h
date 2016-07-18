/***************************************************************************
 *   Copyright (C) 2011 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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


#ifndef KT_SCANTHREAD_H
#define KT_SCANTHREAD_H

#include <QThread>
#include <QMutex>
#include <QStringList>
#include <QUrl>
#include <util/ptrmap.h>
#include "scanfolder.h"


class QDir;

namespace kt
{

    /**
     * Thread which scans directories in the background and looks for torrent files.
     */
    class ScanThread : public QThread
    {
        Q_OBJECT
    public:
        ScanThread();
        virtual ~ScanThread();

        /**
         * Set whether to scan recursively or not
         * @param rec Recursive or not
         */
        void setRecursive(bool rec);

        /**
         * Add a directory to scan.
         * @param url Directory
         * @param recursive Whether or not to scan resursively
         */
        void addDirectory(const QUrl& url, bool recursive);

        /**
         * Stop the scanning thread.
         */
        void stop();

        /**
         * Set the list of folders to scan.
         * @param folders List of folders
         */
        void setFolderList(const QStringList& folders);

    protected:
        virtual void run();

    private:
        void scan(const QUrl& dir, bool recursive);
        bool alreadyLoaded(const QDir& d, const QString& torrent);
        void updateFolders();
        virtual void customEvent(QEvent* ev);

    signals:
        /**
         * Emitted when one or more torrents are found.
         * @param torrents The list of torrents
         */
        void found(const QList<QUrl>& torrent);

    private:
        QMutex mutex;
        QStringList folders;
        bool stop_requested;
        bool recursive;
        bt::PtrMap<QString, ScanFolder> scan_folders;
    };

}

#endif // KT_SCANTHREAD_H
