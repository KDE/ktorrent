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

#include <bcodec/bnode.h>
#include <bcodec/bdecoder.h>
#include <util/error.h>
#include "torrentdbusinterface.h"
#include "engine.h"

using namespace bt;

namespace ktplasma
{

    TorrentDBusInterface::TorrentDBusInterface(const QString& ih, Engine* engine)
        : QObject(engine), info_hash(ih), engine(engine)
    {
        tor = new QDBusInterface("org.ktorrent.ktorrent", "/torrent/" + ih, "org.ktorrent.torrent", QDBusConnection::sessionBus(), this);

        QDBusReply<QString> r = tor->call("name");
        engine->setData(ih, "name", r.isValid() ? r.value() : QString());
        r = tor->call("infoHash");
        engine->setData(ih, "info_hash", r.isValid() ? r.value() : QString());
        QDBusReply<bool> priv = tor->call("isPrivate");
        engine->setData(ih, "private", priv.value());
    }


    TorrentDBusInterface::~TorrentDBusInterface()
    {
    }

    void TorrentDBusInterface::update()
    {
        QDBusReply<QByteArray> r = tor->call("stats");
        if (!r.isValid())
            return;

        QByteArray v = r.value();
        BDecoder dec(v, false, 0);
        BNode* node = 0;
        try
        {
            node = dec.decode();
            if (!node || node->getType() != BNode::DICT)
                throw bt::Error("Root not a dict !");

            BDictNode* dict = (BDictNode*)node;
            const QStringList keys = dict->keys();
            foreach (const QString& key, keys)
            {
                BValueNode* vn = dict->getValue(key);
                if (!vn)
                    continue;

                if (key == "downloaded_chunks" || key == "excluded_chunks")
                {
                    engine->setData(info_hash, key, vn->data().toByteArray());
                }
                else
                {
                    switch (vn->data().getType())
                    {
                    case bt::Value::STRING:
                        engine->setData(info_hash, key, QString::fromUtf8(vn->data().toByteArray()));
                        break;
                    case bt::Value::INT:
                        engine->setData(info_hash, key, vn->data().toInt());
                        break;
                    case bt::Value::INT64:
                        engine->setData(info_hash, key, vn->data().toInt64());
                        break;
                    }
                }
            }
        }
        catch (bt::Error& err)
        {
            engine->setData(info_hash, "update_error", err.toString());
        }

        delete node;
    }
}
