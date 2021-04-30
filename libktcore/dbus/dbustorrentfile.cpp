/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dbustorrentfile.h"

namespace kt
{
DBusTorrentFile::DBusTorrentFile(bt::TorrentFileInterface &file, QObject *parent)
    : QObject(parent)
    , file(file)
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
