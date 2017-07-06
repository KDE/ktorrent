/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                     *
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

#ifndef SCANFOLDER_H
#define SCANFOLDER_H

#include <QObject>
#include <QUrl>
#include <KDirWatch>

namespace kt
{


    class CoreInterface;
    class ScanThread;

    /**
     * Monitors a folder for changes, and passes torrents to load to the TorrentLoadQueue
    */
    class ScanFolder : public QObject
    {
        Q_OBJECT
    public:

        /**
         * Default constructor.
         * @param scanner The ScanThread
         * @param dir The directory
         */
        ScanFolder(ScanThread* scanner, const QUrl &dir, bool recursive);
        ~ScanFolder();

        /**
         * Set if the ScanFolder needs to scan subdirectories recursively
         * @param rec Recursive or not
         */
        void setRecursive(bool rec);

    public slots:
        void scanDir(const QString& path);

    private:
        ScanThread* scanner;
        QUrl scan_directory;
        KDirWatch* watch;
        bool recursive;
    };
}
#endif
