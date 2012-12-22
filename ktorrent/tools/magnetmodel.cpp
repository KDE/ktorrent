/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#include "magnetmodel.h"
#include <KLocalizedString>
#include <KIcon>
#include <QFile>
#include <QTextStream>

#include <util/log.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include <util/error.h>
#include <bcodec/bnode.h>


using namespace bt;

namespace kt
{

    MagnetModel::MagnetModel(QObject* parent) : QAbstractTableModel(parent)
    {

    }

    MagnetModel::~MagnetModel()
    {

    }

    void MagnetModel::download(const bt::MagnetLink& mlink, const MagnetLinkLoadOptions& options)
    {
        addMagnetDownloader(mlink, options, true);
    }

    void MagnetModel::downloadFinished(bt::MagnetDownloader* md, const QByteArray& data)
    {
        kt::MagnetDownloader* ktmd = (kt::MagnetDownloader*)md;
        emit metadataFound(md->magnetLink(), data, ktmd->options);
        int idx = magnet_downloaders.indexOf(ktmd);
        if (idx >= 0)
            removeRow(idx);
    }

    void MagnetModel::addMagnetDownloader(const bt::MagnetLink& mlink, const kt::MagnetLinkLoadOptions& options, bool start)
    {
        foreach (bt::MagnetDownloader* md, magnet_downloaders)
        {
            if (md->magnetLink() == mlink)
                return; // Already downloading, do nothing
        }

        kt::MagnetDownloader* md = new kt::MagnetDownloader(mlink, options, this);
        magnet_downloaders.append(md);
        connect(md, SIGNAL(foundMetadata(bt::MagnetDownloader*, QByteArray)),
                this, SLOT(downloadFinished(bt::MagnetDownloader*, QByteArray)));
        insertRow(magnet_downloaders.size() - 1);
        if (start)
            md->start();
    }

    void MagnetModel::updateMagnetDownloaders()
    {
        foreach (bt::MagnetDownloader* md, magnet_downloaders)
        {
            md->update();
        }

        if (magnet_downloaders.count() > 0)
        {
            // make sure num peers is updated
            emit dataChanged(index(0, 2), index(magnet_downloaders.count() - 1, 2));
        }
    }


    QVariant MagnetModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        bt::MagnetDownloader* md = (bt::MagnetDownloader*)index.internalPointer();
        if (role == Qt::DisplayRole)
        {
            switch (index.column())
            {
            case 0: return displayName(md);
            case 1: return status(md);
            case 2: return md->numPeers();
            default: return QVariant();
            }
        }
        else if (role == Qt::DecorationRole)
        {
            if (index.column() == 0)
                return KIcon("kt-magnet");
        }
        else if (role == Qt::ToolTipRole)
        {
            if (index.column() == 0)
                return md->magnetLink().toString();
        }

        return QVariant();
    }

    QVariant MagnetModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Vertical)
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
            case 0: return i18n("Magnet Link");
            case 1: return i18n("Status");
            case 2: return i18n("Peers");
            default: return QVariant();
            }
        }

        return QVariant();
    }

    int MagnetModel::columnCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return 3;
    }

    int MagnetModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return magnet_downloaders.count();
    }

    QModelIndex MagnetModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (parent.isValid())
            return QModelIndex();

        if (row < 0 || row >= magnet_downloaders.count())
            return QModelIndex();

        return createIndex(row, column, magnet_downloaders[row]);
    }

    bool MagnetModel::insertRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    bool MagnetModel::removeRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        for (int i = 0; i < count; i++)
        {
            kt::MagnetDownloader* md = magnet_downloaders.takeAt(row);
            md->deleteLater();
        }
        endRemoveRows();
        return true;
    }

    void MagnetModel::removeMagnetDownloader(kt::MagnetDownloader* md)
    {
        int idx = magnet_downloaders.indexOf(md);
        if (idx != -1)
            removeRow(idx);
    }

    QString MagnetModel::displayName(const bt::MagnetDownloader* md) const
    {
        if (md->magnetLink().displayName().isEmpty())
            return md->magnetLink().toString();
        else
            return md->magnetLink().displayName();
    }

    QString MagnetModel::status(const bt::MagnetDownloader* md) const
    {
        if (md->running())
            return i18n("Downloading");
        else
            return i18n("Stopped");
    }

    void MagnetModel::start(const QModelIndex& idx)
    {
        if (!idx.isValid())
            return;

        bt::MagnetDownloader* md = (bt::MagnetDownloader*)idx.internalPointer();
        if (!md || md->running())
            return;

        md->start();
        emit dataChanged(idx, idx);
    }

    void MagnetModel::stop(const QModelIndex& idx)
    {
        if (!idx.isValid())
            return;

        bt::MagnetDownloader* md = (bt::MagnetDownloader*)idx.internalPointer();
        if (!md || !md->running())
            return;

        md->stop();
        emit dataChanged(idx, idx);
    }

    void MagnetModel::loadMagnets(const QString& file)
    {
        QFile fptr(file);
        if (!fptr.open(QIODevice::ReadOnly))
        {
            Out(SYS_GEN | LOG_NOTICE) << "Failed to open " << file << " : " << fptr.errorString() << endl;
            return;
        }

        QByteArray magnet_data = fptr.readAll();
        if (magnet_data.size() == 0)
            return;

        BDecoder decoder(magnet_data, 0, false);
        BNode* node = 0;
        try
        {
            node = decoder.decode();
            if (!node || node->getType() != BNode::LIST)
                throw bt::Error("Corrupted magnet file");

            BListNode* ml = (BListNode*)node;
            for (Uint32 i = 0; i < ml->getNumChildren(); i++)
            {
                BDictNode* dict = ml->getDict(i);
                bt::MagnetLink mlink(dict->getString("magnet", 0));
                MagnetLinkLoadOptions options;
                bool running = dict->getInt("running") == 1;
                options.silently = dict->getInt("silent") == 1;

                if (dict->keys().contains("group"))
                    options.group = dict->getString("group", 0);
                if (dict->keys().contains("location"))
                    options.location = dict->getString("location", 0);
                if (dict->keys().contains("move_on_completion"))
                    options.move_on_completion = dict->getString("move_on_completion", 0);

                addMagnetDownloader(mlink, options, running);
            }
        }
        catch (bt::Error& err)
        {
            Out(SYS_GEN | LOG_NOTICE) << "Failed to load " << file << " : " << err.toString() << endl;
        }
        delete node;
    }

    void MagnetModel::saveMagnets(const QString& file)
    {
        bt::File fptr;
        if (!fptr.open(file, "wb"))
        {
            Out(SYS_GEN | LOG_NOTICE) << "Failed to open " << file << " : " << fptr.errorString() << endl;
            return;
        }

        BEncoder enc(&fptr);
        enc.beginList();
        foreach (kt::MagnetDownloader* md, magnet_downloaders)
        {
            enc.beginDict();
            enc.write("magnet", md->magnetLink().toString());
            enc.write("running", md->running());
            enc.write("silent", md->options.silently);
            enc.write("group", md->options.group);
            enc.write("location", md->options.location);
            enc.write("move_on_completion", md->options.move_on_completion);
            enc.end();
        }
        enc.end();
    }
}
