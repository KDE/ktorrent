/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
 *   Copyright (C) 2018 by Emmanuel Eytan                                  *
 *   eje211@gmail.com                                                      *
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

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMap>
#include <QString>
#include <iostream>

#include "torrentlistgenerator.h"
#include <util/sha1hash.h>
#include <torrent/queuemanager.h>
#include <interfaces/coreinterface.h>

using namespace bt;
using namespace std;

namespace kt
{

    TorrentListGenerator::TorrentListGenerator(CoreInterface* core): core(core)
    {
    }


    TorrentListGenerator::~TorrentListGenerator()
    {
    }


    void TorrentListGenerator::get(char ** str_p, int * size)
    {
        kt::QueueManager* qman = core->getQueueManager();
        kt::QueueManager::iterator i = qman->begin();
        QJsonArray array = QJsonArray();
        while (i != qman->end())
        {
            bt::TorrentInterface* ti = *i;
            const bt::TorrentStats& s = ti->getStats();

            QJsonObject json = QJsonObject() ;
            json.insert(QString::fromLocal8Bit("name"), ti->getDisplayName());
            json.insert(QString::fromLocal8Bit("infoHash"), ti->getInfoHash().toString());
            json.insert(QString::fromLocal8Bit("status"), s.status);
            json.insert(QString::fromLocal8Bit("timeAdded"), s.time_added.toMSecsSinceEpoch());            
            json.insert(QString::fromLocal8Bit("bytesDownloaded"), (double) s.bytes_downloaded);
            json.insert(QString::fromLocal8Bit("bytesUploaded"), (double) s.bytes_uploaded);
            json.insert(QString::fromLocal8Bit("totalBytes"), (double) s.total_bytes);
            json.insert(QString::fromLocal8Bit("totalBytesToDownload"), (double) s.total_bytes_to_download);
            json.insert(QString::fromLocal8Bit("downloadRate"), (double) s.download_rate);
            json.insert(QString::fromLocal8Bit("uploadRate"), (double) s.upload_rate);
            json.insert(QString::fromLocal8Bit("numPeers"), (double) s.num_peers);
            json.insert(QString::fromLocal8Bit("seeders"), (double) s.seeders_connected_to);
            json.insert(QString::fromLocal8Bit("seedersTotal"), (double) s.seeders_total);
            json.insert(QString::fromLocal8Bit("leechers"), (double) s.leechers_connected_to);
            json.insert(QString::fromLocal8Bit("leechersTotal"), (double) s.leechers_total);
            json.insert(QString::fromLocal8Bit("running"), s.running);
            json.insert(QString::fromLocal8Bit("numFiles"), (double) ti->getNumFiles());
            array.append(json);
            i++;
        }
        QJsonObject message = QJsonObject();
        message.insert(QString::fromLocal8Bit("torrents"), array);
        QJsonDocument doc = QJsonDocument(message);
        *str_p = doc.toJson().data();
        *size = doc.toJson().size();
    }

    void TorrentListGenerator::post(char * json) {
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray((const char *) json));
        QMap<QString, QVariant> qJson = doc.toVariant().toMap();
        
        QString action = qJson[QString::fromLatin1("type")].toString();

        if (action.isEmpty() || action.isNull()) {
            return;
        }
        
        if (action == QString::fromLatin1("magnet")) {
            QString magnet = qJson[QString::fromLatin1("magnet")].toString();
            if (magnet.isEmpty() || magnet.isNull()) {
                return;
            }
            core->loadSilently(QUrl(magnet), QString());
            return;
        }
        
        QString hash = qJson[QString::fromLatin1("hash")].toString();
        if (hash.isEmpty() || hash.isNull()) {
            return;
        }

        TorrentInterface * ti = getTorrentFromHash(hash);
        if (action == QString::fromLatin1("start")) {
            if (ti->getStats().paused) {
                ti->unpause();
            } else {
                ti->start();
            }
        }
        if (action == QString::fromLatin1("pause")) {
            ti->pause();
        }
        if (action == QString::fromLatin1("remove")) {
            core->remove(ti, false);
        }
    }
    
    const SHA1Hash * TorrentListGenerator::QStringToSHA1Hash(QString qString) {
        Uint8 hashNum[20] = {};
        hex2bin((const char *) qString.toLatin1().data(), hashNum);
        const SHA1Hash * hash = new SHA1Hash(hashNum);
        return hash;
    }
    
    TorrentInterface * TorrentListGenerator::getTorrentFromHash(QString qHash) {
        const SHA1Hash * hash = QStringToSHA1Hash(qHash);
        kt::QueueManager* qman = core->getQueueManager();
        kt::QueueManager::iterator i = qman->begin();
        while (i != qman->end())
        {
            bt::TorrentInterface* ti = *i;
            const bt::TorrentStats& s = ti->getStats();
            if (ti->getInfoHash() == *hash) {
                return ti;
            }
            i++;
        }
        return NULL;
    }
    
    /**
     * Shameless ripped from: https://stackoverflow.com/a/17261928/313273
     */
    Uint8 TorrentListGenerator::char2int(char input)
    {
        if(input >= '0' && input <= '9')
            return (Uint8) input - '0';
        if(input >= 'A' && input <= 'F')
            return (Uint8) input - 'A' + 10;
        if(input >= 'a' && input <= 'f')
            return (Uint8) input - 'a' + 10;
        throw std::invalid_argument("Invalid input string");
    }

    /**
     * Shamelessly ripped from: https://stackoverflow.com/a/17261928/313273
     * This function assumes src to be a zero terminated sanitized string with
     * an even number of [0-9a-f] characters, and target to be sufficiently large.
     */
    void TorrentListGenerator::hex2bin(const char * src, Uint8 * target)
    {
        while(*src && src[1])
        {
            *(target++) = char2int(*src) * 16 + char2int(src[1]);
            src += 2;
        }
    }
}
