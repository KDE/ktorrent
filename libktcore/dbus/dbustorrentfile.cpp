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

#include "dbustorrentfile.h"

namespace kt
{

    DBusTorrentFile::DBusTorrentFile(bt::TorrentFileInterface& file, QObject* parent)
        : QObject(parent), file(file)
    {
    }


    DBusTorrentFile::~DBusTorrentFile()
    {
    }

    QString DBusTorrentFile::path() const
    {
        return file.getPath();
    }

    QString DBusTorrentFile::pathOnDisk() const
    {
        return file.getPathOnDisk();
    }

    qulonglong DBusTorrentFile::size() const
    {
        return file.getSize();
    }

    int DBusTorrentFile::priority() const
    {
        return file.getPriority();
    }

    void DBusTorrentFile::setPriority(int prio)
    {
        if (prio > 60 || prio < 10)
            return;

        if (prio % 10 != 0)
            return;

        file.setPriority((bt::Priority)prio);
    }

    void DBusTorrentFile::setDoNotDownload(bool dnd)
    {
        file.setDoNotDownload(dnd);
    }

    int DBusTorrentFile::firstChunk() const
    {
        return file.getFirstChunk();
    }

    int DBusTorrentFile::lastChunk() const
    {
        return file.getLastChunk();
    }

    double DBusTorrentFile::percentage() const
    {
        return file.getDownloadPercentage();
    }

    bool DBusTorrentFile::isMultiMedia() const
    {
        return file.isMultimedia();
    }
}
