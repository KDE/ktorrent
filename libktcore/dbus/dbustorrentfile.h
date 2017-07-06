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

#ifndef KTDBUSTORRENTFILE_H
#define KTDBUSTORRENTFILE_H

#include <QObject>
#include <interfaces/torrentfileinterface.h>


namespace kt
{

    /**
        @author
    */
    class DBusTorrentFile : public QObject
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.ktorrent.torrentfile")
    public:
        DBusTorrentFile(bt::TorrentFileInterface& file, QObject* parent);
        ~DBusTorrentFile();

    public Q_SLOTS:
        Q_SCRIPTABLE QString path() const;
        Q_SCRIPTABLE QString pathOnDisk() const;
        Q_SCRIPTABLE qulonglong size() const;
        Q_SCRIPTABLE int priority() const;
        Q_SCRIPTABLE void setPriority(int prio);
        Q_SCRIPTABLE int firstChunk() const;
        Q_SCRIPTABLE int lastChunk() const;
        Q_SCRIPTABLE double percentage() const;
        Q_SCRIPTABLE bool isMultiMedia() const;
        Q_SCRIPTABLE void setDoNotDownload(bool dnd);

    private:
        bt::TorrentFileInterface& file;
    };

}

#endif
