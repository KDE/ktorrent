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

#include "torrentloadqueue.h"

#include <QDir>
#include <QFileInfo>
#include <QDateTime>

#include <KLocalizedString>
#include <KIO/Job>

#include <bcodec/bnode.h>
#include <bcodec/bdecoder.h>
#include <interfaces/coreinterface.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <util/log.h>
#include "scanfolderpluginsettings.h"



namespace kt
{
    TorrentLoadQueue::TorrentLoadQueue(CoreInterface* core, QObject* parent)
        : QObject(parent),
          core(core)
    {
        connect(&timer, &QTimer::timeout, this, &TorrentLoadQueue::loadOne);
        timer.setSingleShot(true);
    }

    TorrentLoadQueue::~TorrentLoadQueue()
    {

    }

    void TorrentLoadQueue::add(const QUrl& url)
    {
        to_load.append(url);
        if (!timer.isActive())
            timer.start(1000);
    }

    void TorrentLoadQueue::add(const QList<QUrl>& urls)
    {
        to_load.append(urls);
        if (!timer.isActive())
            timer.start(1000);
    }


    bool TorrentLoadQueue::validateTorrent(const QUrl& url, QByteArray& data)
    {
        // try to decode file, if it is syntactically correct, we can try to load it
        QFile fptr(url.toLocalFile());
        if (!fptr.open(QIODevice::ReadOnly))
            return false;

        try
        {
            data = fptr.readAll();

            bt::BDecoder dec(data, false);
            bt::BNode* n = dec.decode();
            if (n)
            {
                // valid node, so file is complete
                delete n;
                return true;
            }
            else
            {
                // decoding failed so incomplete
                return false;
            }
        }
        catch (...)
        {
            // any error means shit happened and the file is incomplete
            return false;
        }
    }

    void TorrentLoadQueue::loadOne()
    {
        if (to_load.isEmpty())
            return;

        QUrl url = to_load.takeFirst();

        QByteArray data;
        if (validateTorrent(url, data))
        {
            // Load it
            load(url, data);
        }
        else
        {
            // Not valid, so two options:
            // - not a torrent
            // - incomplete torrent, still being written
            // We use the last modified time to determine this
            if (QFileInfo(url.toLocalFile()).lastModified().secsTo(QDateTime::currentDateTime()) < 2)
            {
                // Still being written, lets try again later
                to_load.append(url);
            }
        }

        if (!to_load.isEmpty())
            timer.start(1000);
    }

    void TorrentLoadQueue::load(const QUrl& url, const QByteArray& data)
    {
        bt::Out(SYS_SNF | LOG_NOTICE) << "ScanFolder: loading " << url.toDisplayString() << bt::endl;
        QString group;
        if (ScanFolderPluginSettings::addToGroup())
            group = ScanFolderPluginSettings::group();

        if (ScanFolderPluginSettings::openSilently())
            core->loadSilently(data, url, group, QString());
        else
            core->load(data, url, group, QString());

        loadingFinished(url);
    }

    void TorrentLoadQueue::loadingFinished(const QUrl& url)
    {
        QString name = url.fileName();
        QString dirname = QFileInfo(url.toLocalFile()).absolutePath();
        if (!dirname.endsWith(bt::DirSeparator()))
            dirname += bt::DirSeparator();

        switch (action)
        {
        case DeleteAction:
            // If torrent has it's hidden complement - remove it too.
            if (bt::Exists(dirname + QLatin1Char('.') + name))
                bt::Delete(dirname + QLatin1Char('.') + name, true);

            bt::Delete(url.toLocalFile(), true);
            break;
        case MoveAction:
            // If torrent has it's hidden complement - remove it too.
            if (bt::Exists(dirname + QLatin1Char('.') + name))
                bt::Delete(dirname + QLatin1Char('.') + name, true);

            if (!bt::Exists(dirname + i18nc("folder name part", "loaded")))
                bt::MakeDir(dirname + i18nc("folder name part", "loaded"), true);

            KIO::file_move(url,
                           QUrl::fromLocalFile(dirname + i18nc("folder name part", "loaded") + bt::DirSeparator() + name),
                           -1,
                           KIO::HideProgressInfo | KIO::Overwrite);
            break;
        case DefaultAction:
            QFile f(dirname + QLatin1Char('.') + name);
            f.open(QIODevice::WriteOnly);
            f.close();
            break;
        }
    }
}
