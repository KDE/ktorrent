/***************************************************************************
 *   Copyright (C) 2020 by Alexander Trufanov                              *
 *   trufanovan@gmail.com                                                  *
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

#ifndef SCANFORLOSTFILESTHREAD_H
#define SCANFORLOSTFILESTHREAD_H

#include <QThread>
#include <QSet>

namespace kt
{

    class CoreInterface;

        /**
        * ScanForLostFiles working thread. It:
        * 1. Lists the folder content with QDirIterator
        * 2. Lists the files that belongs to torrents
        * 3. Substracts the results of 2nd step from results of 1st step
        * 4. Returns resulting file tree as a set of filepaths by emitting filterReady signal
        */

    class ScanForLostFilesThread : public QThread
    {
        Q_OBJECT
    public:
        /**
         * Set the list of folders to scan.
         * @param folder A folder whose content shall be displayed
         * @param core   A core interface pointer to get list of torrent files
         */
        ScanForLostFilesThread(const QString& folder, CoreInterface* core, QObject *parent = nullptr);
    protected:
        void run() override;

    Q_SIGNALS:
        /**
         * Emitted when filter generation is complete.
         * @param filter Pointer to set of filepaths that are not belong to any torrent
         */
        void filterReady(QSet<QString>* filter);
    private:
        QString m_root_folder;
        CoreInterface* m_core;
    };

}
#endif // SCANFORLOSTFILESTHREAD_H
