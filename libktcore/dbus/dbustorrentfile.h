/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    DBusTorrentFile(bt::TorrentFileInterface &file, QObject *parent);
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
    bt::TorrentFileInterface &file;
};

}

#endif
