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

#include <algorithm>

#include <QApplication>
#include <QDataStream>
#include <QFont>
#include <QIcon>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>

#include <util/log.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include "downloadordermodel.h"

using namespace bt;

namespace kt
{

    DownloadOrderModel::DownloadOrderModel(bt::TorrentInterface* tor, QObject* parent) : QAbstractListModel(parent), tor(tor)
    {
        for (Uint32 i = 0; i < tor->getNumFiles(); i++)
        {
            order.append(i);
        }
    }


    DownloadOrderModel::~DownloadOrderModel()
    {
    }

    int DownloadOrderModel::rowCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return tor->getNumFiles();
        else
            return 0;
    }

    QVariant DownloadOrderModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        Uint32 idx = order.at(index.row());
        if (idx >= tor->getNumFiles())
            return QVariant();


        switch (role)
        {
        case Qt::DisplayRole:
            return tor->getTorrentFile(idx).getUserModifiedPath();
        case Qt::DecorationRole:
            return QIcon::fromTheme(QMimeDatabase().mimeTypeForFile(tor->getTorrentFile(idx).getPath()).iconName());
        case Qt::FontRole:
            if (!current_search_text.isEmpty() && tor->getTorrentFile(idx).getUserModifiedPath().contains(current_search_text, Qt::CaseInsensitive))
            {
                QFont font = QApplication::font();
                font.setBold(true);
                return font;
            }
        default:
            return QVariant();
        }
    }

    QModelIndex DownloadOrderModel::find(const QString& text)
    {
        beginResetModel();
        current_search_text = text;
        for (Uint32 i = 0; i < tor->getNumFiles(); i++)
        {
            if (tor->getTorrentFile(i).getUserModifiedPath().contains(current_search_text, Qt::CaseInsensitive))
            {
                endResetModel();
                return index(i);
            }
        }

        endResetModel();
        return QModelIndex();
    }

    void DownloadOrderModel::clearHighLights()
    {
        beginResetModel();
        current_search_text.clear();
        endResetModel();
    }

    Qt::ItemFlags DownloadOrderModel::flags(const QModelIndex& index) const
    {
        Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

        if (index.isValid())
            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
        else
            return Qt::ItemIsDropEnabled | defaultFlags;
    }

    Qt::DropActions DownloadOrderModel::supportedDropActions() const
    {
        return Qt::CopyAction | Qt::MoveAction;
    }

    QStringList DownloadOrderModel::mimeTypes() const
    {
        QStringList types;
        types << QStringLiteral("application/octet-stream");
        return types;
    }

    QMimeData* DownloadOrderModel::mimeData(const QModelIndexList& indexes) const
    {
        QMimeData* mimeData = new QMimeData();
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        QList<Uint32> files;

        foreach (const QModelIndex& index, indexes)
        {
            if (index.isValid())
            {
                files.append(order.at(index.row()));
            }
        }
        out << files;
        mimeData->setData(QStringLiteral("application/octet-stream"), data);
        return mimeData;
    }

    bool DownloadOrderModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
    {
        Q_UNUSED(column);
        if (action == Qt::IgnoreAction)
            return true;

        if (!data->hasFormat(QStringLiteral("application/octet-stream")))
            return false;

        int begin_row;
        if (row != -1)
            begin_row = row;
        else if (parent.isValid())
            begin_row = parent.row();
        else
            begin_row = rowCount(QModelIndex());

        QByteArray file_data = data->data(QStringLiteral("application/octet-stream"));
        QDataStream in(&file_data, QIODevice::ReadOnly);
        QList<Uint32> files;
        in >> files;

        // remove all files from order which are in the dragged list
        int r = 0;
        for (QList<Uint32>::iterator i = order.begin(); i != order.end();)
        {
            if (files.contains(*i))
            {
                if (r < begin_row)   // if we remove something before the begin row, the row to insert decreases
                    begin_row--;

                i = order.erase(i);
            }
            else
                i++;

            r++;
        }

        // reinsert dragged files
        foreach (Uint32 file, files)
        {
            order.insert(begin_row, file);
            begin_row++;
        }
        return true;
    }


    void DownloadOrderModel::moveUp(int row, int count)
    {
        if (row == 0)
            return;

        for (int i = 0; i < count; i++)
        {
            order.swap(row + i, row + i - 1);
        }

        emit dataChanged(createIndex(row - 1, 0), createIndex(row + count, 0));
    }

    void DownloadOrderModel::moveTop(int row, int count)
    {
        if (row == 0)
            return;

        QList<Uint32> tmp;
        for (int i = 0; i < count; i++)
        {
            tmp.append(order.takeAt(row));
        }

        beginResetModel();
        order = tmp + order;
        endResetModel();
    }

    void DownloadOrderModel::moveDown(int row, int count)
    {
        if (row + count >= (int)tor->getNumFiles())
            return;

        for (int i = count - 1; i >= 0; i--)
        {
            order.swap(row + i, row + i + 1);
        }

        emit dataChanged(createIndex(row, 0), createIndex(row + count + 1, 0));
    }

    void DownloadOrderModel::moveBottom(int row, int count)
    {
        if (row + count >= (int)tor->getNumFiles())
            return;

        QList<Uint32> tmp;
        for (int i = 0; i < count; i++)
        {
            tmp.append(order.takeAt(row));
        }

        beginResetModel();
        order = order + tmp;
        endResetModel();
    }

    struct NameCompare
    {
        NameCompare(bt::TorrentInterface* tor) : tor(tor)
        {}

        bool operator()(Uint32 a, Uint32 b)
        {
            return tor->getTorrentFile(a).getUserModifiedPath() < tor->getTorrentFile(b).getUserModifiedPath();
        }

        bt::TorrentInterface* tor;
    };

    void DownloadOrderModel::sortByName()
    {
        beginResetModel();
        std::sort(order.begin(), order.end(), NameCompare(tor));
        endResetModel();
    }

    struct AlbumTrackCompare
    {
        AlbumTrackCompare(bt::TorrentInterface* tor) : tor(tor)
        {}

        int getTrack(const QString& title)
        {
            QRegExp exp(QLatin1String(".*(\\d+)\\s.*\\.\\w*"), Qt::CaseInsensitive);
            int pos = exp.indexIn(title);
            if (pos > -1)
            {
                QString track = exp.cap(1);
                bool ok = false;
                int track_number = track.toInt(&ok);
                if (ok)
                    return track_number;
            }

            return -1;
        }

        bool operator()(Uint32 a, Uint32 b)
        {
            QString a_path = tor->getTorrentFile(a).getUserModifiedPath();
            QString b_path = tor->getTorrentFile(b).getUserModifiedPath();

            int ta = getTrack(a_path);
            int tb = getTrack(b_path);
            if (ta < 0 && tb < 0)
                return a_path < b_path;
            else if (ta < 0)
                return false;
            else if (tb < 0)
                return true;
            else
                return ta < tb;
        }

        bt::TorrentInterface* tor;
    };

    void DownloadOrderModel::sortByAlbumTrackOrder()
    {
        beginResetModel();
        std::sort(order.begin(), order.end(), AlbumTrackCompare(tor));
        endResetModel();
    }

    struct SeasonEpisodeCompare
    {
        SeasonEpisodeCompare(bt::TorrentInterface* tor) : tor(tor)
        {}

        bool getSeasonAndEpisode(const QString& title, int& season, int& episode)
        {
            QStringList se_formats;
            se_formats << QStringLiteral("(\\d+)x(\\d+)")
                       << QStringLiteral("S(\\d+)E(\\d+)")
                       << QStringLiteral("(\\d+)\\.(\\d+)")
                       << QStringLiteral("S(\\d+)\\.E(\\d+)")
                       << QStringLiteral("Season\\s(\\d+).*Episode\\s(\\d+)");

            foreach (const QString& format, se_formats)
            {
                QRegExp exp(format, Qt::CaseInsensitive);
                int pos = exp.indexIn(title);
                if (pos > -1)
                {
                    QString s = exp.cap(1); // Season
                    QString e = exp.cap(2);  // Episode
                    bool ok = false;
                    season = s.toInt(&ok);
                    if (!ok)
                        continue;

                    episode = e.toInt(&ok);
                    if (!ok)
                        continue;

                    return true;
                }
            }

            return false;
        }

        bool operator()(Uint32 a, Uint32 b)
        {
            QString a_path = tor->getTorrentFile(a).getUserModifiedPath();
            QString b_path = tor->getTorrentFile(b).getUserModifiedPath();
            int a_season = 0, a_episode = 0;
            int b_season = 0, b_episode = 0;
            bool a_has_se = getSeasonAndEpisode(a_path, a_season, a_episode);
            bool b_has_se = getSeasonAndEpisode(b_path, b_season, b_episode);
            if (a_has_se && b_has_se)
            {
                if (a_season == b_season)
                    return a_episode < b_episode;
                else
                    return a_season < b_season;
            }
            else if (a_has_se && !b_has_se)
            {
                return true;
            }
            else if (!a_has_se && b_has_se)
            {
                return false;
            }
            else
            {
                return a_path < b_path;
            }
        }

        bt::TorrentInterface* tor;
    };

    void DownloadOrderModel::sortBySeasonsAndEpisodes()
    {
        beginResetModel();
        std::sort(order.begin(), order.end(), SeasonEpisodeCompare(tor));
        endResetModel();
    }
}
