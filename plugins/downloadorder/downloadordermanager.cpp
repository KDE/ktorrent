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

#include <QFile>
#include <QTextStream>

#include <util/log.h>
#include <util/fileops.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "downloadordermanager.h"

using namespace bt;

namespace kt
{

    DownloadOrderManager::DownloadOrderManager(bt::TorrentInterface* tor) : tor(tor)
    {
        current_normal_priority_file = current_high_priority_file = tor->getNumFiles();
    }

    DownloadOrderManager::~DownloadOrderManager()
    {
    }

    void DownloadOrderManager::save()
    {
        if (!enabled())
            return;

        QFile fptr(tor->getTorDir() + QStringLiteral("download_order"));
        if (!fptr.open(QIODevice::WriteOnly))
        {
            Out(SYS_DIO | LOG_IMPORTANT) << "Cannot open download_order file of " << tor->getDisplayName() << " : " << fptr.errorString() <<  endl;
            return;
        }

        QTextStream out(&fptr);
        foreach (Uint32 file, order)
            out << file << endl;
    }

    void DownloadOrderManager::load()
    {
        if (!bt::Exists(tor->getTorDir() + QStringLiteral("download_order")))
            return;

        QFile fptr(tor->getTorDir() + QStringLiteral("download_order"));
        if (!fptr.open(QIODevice::ReadOnly))
        {
            Out(SYS_DIO | LOG_NOTICE) << "Cannot open download_order file of " << tor->getDisplayName() << " : " << fptr.errorString() << endl;
            return;
        }

        QTextStream in(&fptr);
        while (!in.atEnd())
        {
            QString file = in.readLine();
            bool ok = false;
            Uint32 idx = file.toUInt(&ok);
            if (ok && idx < tor->getNumFiles())
                order.append(idx);
        }

        // make sure all files are in the order
        for (Uint32 i = 0; i < tor->getNumFiles(); i++)
            if (!order.contains(i))
                order.append(i);
    }

    Uint32 DownloadOrderManager::nextIncompleteFile()
    {
        // Look for the next file in the order which is not 100 % complete
        foreach (Uint32 file, order)
        {
            // skip file if it is complete
            if (qAbs(100.0f - tor->getTorrentFile(file).getDownloadPercentage()) < 0.01)
                continue;

            // skip excluded or only seed files
            if (tor->getTorrentFile(file).getPriority() < LAST_PRIORITY)
                continue;

            // we have found the incomplete file
            return file;
        }
        return tor->getNumFiles();
    }

    void DownloadOrderManager::chunkDownloaded(bt::TorrentInterface* me, Uint32 chunk)
    {
        if (!enabled() || tor->getStats().completed || tor != me)
            return;

        bt::TorrentFileInterface& high_priority_file = tor->getTorrentFile(current_high_priority_file);
        bool in_high_priority_file_range = chunk >= high_priority_file.getFirstChunk() && chunk <= high_priority_file.getLastChunk();
        bt::TorrentFileInterface& normal_priority_file = tor->getTorrentFile(current_normal_priority_file);
        bool in_normal_priority_file_range = chunk >= normal_priority_file.getFirstChunk() && chunk <= normal_priority_file.getLastChunk();
        if (in_high_priority_file_range || in_normal_priority_file_range)
        {
            // Check if high or normal are complete
            if (qAbs(100.0f - high_priority_file.getDownloadPercentage()) < 0.01 || qAbs(100.0f - normal_priority_file.getDownloadPercentage()) < 0.01)
            {
                update();
            }
        }
    }

    void DownloadOrderManager::update()
    {
        if (!enabled() || tor->getStats().completed)
            return;

        Uint32 next_file = nextIncompleteFile();
        if (next_file >= tor->getNumFiles())
            return;

        if (next_file != current_high_priority_file)
            Out(SYS_DIO | LOG_NOTICE) << "DownloadOrderPlugin: next file to download is " << tor->getTorrentFile(next_file).getUserModifiedPath() << endl;

        bool normal_found = false;
        bool high_found = false;
        // set the priority of the file to FIRST and all the other files to NORMAL
        foreach (Uint32 file, order)
        {
            TorrentFileInterface& tf = tor->getTorrentFile(file);
            if (tf.getPriority() < LAST_PRIORITY)
                continue;

            if (file == next_file)
            {
                tf.setPriority(FIRST_PRIORITY);
                high_found = true;
            }
            else if (!normal_found && high_found)
            {
                // the file after the high prio file is set to normal
                // so that when the high prio file is finished the selector
                // will select it before we can set a new high prio file
                tf.setPriority(NORMAL_PRIORITY);
                normal_found = true;
                current_normal_priority_file = file;
            }
            else
                tf.setPriority(LAST_PRIORITY);
        }
        current_high_priority_file = next_file;
    }

    void DownloadOrderManager::enable()
    {
        if (enabled())
            return;

        for (Uint32 i = 0; i < tor->getNumFiles(); i++)
        {
            order.append(i);
        }
    }

    void DownloadOrderManager::disable()
    {
        order.clear();
        if (bt::Exists(tor->getTorDir() + QStringLiteral("download_order")))
            bt::Delete(tor->getTorDir() + QStringLiteral("download_order"), true);
    }

}
