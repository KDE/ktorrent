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

#include "playlist.h"

#include <algorithm>

#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QMimeData>
#include <QStringList>
#include <QTextStream>
#include <QTime>
#include <QUrl>

#include <KLocalizedString>

#include <taglib/tag.h>
#include <util/log.h>
#include "mediaplayer.h"


using namespace bt;

namespace kt
{
    PlayList::PlayList(kt::MediaFileCollection* collection, kt::MediaPlayer* player, QObject* parent)
        : QAbstractItemModel(parent),
          collection(collection),
          player(player)
    {
        connect(player, &MediaPlayer::playing, this, &PlayList::onPlaying);
    }

    PlayList::~PlayList()
    {
    }

    void PlayList::addFile(const MediaFileRef& file)
    {
        QByteArray name = QFile::encodeName(file.path());
        TagLib::FileRef* ref = new TagLib::FileRef(name.data(), true, TagLib::AudioProperties::Fast);
        files.append(qMakePair(file, ref));
        insertRow(files.count() - 1);
    }

    void PlayList::removeFile(const MediaFileRef& file)
    {
        int i = 0;
        foreach (const PlayListItem& item, files)
        {
            if (item.first == file)
            {
                removeRow(i);
                break;
            }
            i++;
        }
    }

    MediaFileRef PlayList::fileForIndex(const QModelIndex& index) const
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= files.count())
            return MediaFileRef(QString());
        else
            return files.at(index.row()).first;
    }

    void PlayList::clear()
    {
        beginResetModel();
        files.clear();
        endResetModel();
    }



    QVariant PlayList::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Vertical || role != Qt::DisplayRole)
            return QVariant();

        switch (section)
        {
        case 0: return i18n("Title");
        case 1: return i18n("Artist");
        case 2: return i18n("Album");
        case 3: return i18n("Length");
        case 4: return i18n("Year");
        default: return QVariant();
        }
    }

    QVariant PlayList::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::UserRole && role != Qt::DecorationRole))
            return QVariant();

        const PlayListItem& item = files.at(index.row());
        const MediaFileRef& file = item.first;
        const TagLib::FileRef* ref = item.second;
        if (!ref)
        {
            QByteArray name = QFile::encodeName(file.path());
            files[index.row()].second = new TagLib::FileRef(name.data(), true, TagLib::AudioProperties::Fast);
            ref = item.second;
        }

        if (!ref || ref->isNull())
        {
            if (index.column() == 0)
                return QFileInfo(file.path()).fileName();
            else
                return QVariant();
        }

        TagLib::Tag* tag = ref->tag();
        if (!tag)
        {
            if (index.column() == 0)
                return QFileInfo(file.path()).fileName();
            else
                return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::UserRole)
        {
            switch (index.column())
            {
            case 0:
            {
                QString title = TStringToQString(tag->title());
                return title.isEmpty() ? QFileInfo(file.path()).fileName() : title;
            }
            case 1: return TStringToQString(tag->artist());
            case 2: return TStringToQString(tag->album());
            case 3:
                if (role == Qt::UserRole)
                {
                    return ref->audioProperties()->length();
                }
                else
                {
                    QTime t(0, 0);
                    t = t.addSecs(ref->audioProperties()->length());
                    return t.toString(QStringLiteral("m:ss"));
                }
            case 4: return tag->year() == 0 ? QVariant() : tag->year();
            default: return QVariant();
            }
        }

        if (role == Qt::DecorationRole && index.column() == 0)
        {
            if (file == player->getCurrentSource())
                return QIcon::fromTheme(QStringLiteral("arrow-right"));
        }

        return QVariant();
    }

    int PlayList::columnCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return 5;
    }

    int PlayList::rowCount(const QModelIndex& parent) const
    {
        return parent.isValid() ? 0 : files.count();
    }

    QModelIndex PlayList::parent(const QModelIndex& child) const
    {
        Q_UNUSED(child);
        return QModelIndex();
    }

    QModelIndex PlayList::index(int row, int column, const QModelIndex& parent) const
    {
        if (parent.isValid())
            return QModelIndex();
        else
            return createIndex(row, column);
    }

    Qt::DropActions PlayList::supportedDropActions() const
    {
        return Qt::CopyAction | Qt::MoveAction;
    }

    Qt::ItemFlags PlayList::flags(const QModelIndex& index) const
    {
        Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

        if (index.isValid())
            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
        else
            return Qt::ItemIsDropEnabled | defaultFlags;
    }

    QStringList PlayList::mimeTypes() const
    {
        QStringList types;
        types << QStringLiteral("text/uri-list");
        return types;
    }

    QMimeData* PlayList::mimeData(const QModelIndexList& indexes) const
    {
        dragged_rows.clear();
        QMimeData* data = new QMimeData();
        QList<QUrl> urls;
        for (QModelIndex index : indexes)
        {
            if (index.isValid() && index.column() == 0)
            {
                urls << QUrl::fromLocalFile(files.at(index.row()).first.path());
                dragged_rows.append(index.row());
            }
        }

        data->setUrls(urls);
        return data;
    }

    bool PlayList::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
    {
        if (action == Qt::IgnoreAction)
            return true;

        QList<QUrl> urls = data->urls();
        if (urls.count() == 0 || column > 0)
            return false;

        if (row == -1)
            row = parent.row();

        if (row == -1)
            row = rowCount(QModelIndex());

        // Remove dragged rows if there are any
        std::sort(dragged_rows.begin(), dragged_rows.end());
        int nr = 0;
        for (int r : qAsConst(dragged_rows))
        {
            r -= nr;
            removeRow(r);
            nr++;
        }

        row -= nr;

        for (const QUrl& url : qAsConst(urls))
        {
            PlayListItem item = qMakePair(collection->find(url.toLocalFile()), (TagLib::FileRef*)0);
            files.insert(row, item);
        }
        insertRows(row, urls.count(), QModelIndex());
        dragged_rows.clear();
        itemsDropped();
        return true;
    }

    bool PlayList::insertRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    bool PlayList::removeRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        for (int i = 0; i < count; i++)
            files.removeAt(i + row);
        endRemoveRows();
        return true;
    }

    void PlayList::save(const QString& file)
    {
        QFile fptr(file);
        if (!fptr.open(QIODevice::WriteOnly))
        {
            Out(SYS_GEN | LOG_NOTICE) << "Failed to open file " << file << endl;
            return;
        }

        QTextStream out(&fptr);
        for (const PlayListItem& f : qAsConst(files))
            out << f.first.path() << endl;
    }

    void PlayList::load(const QString& file)
    {
        QFile fptr(file);
        if (!fptr.open(QIODevice::ReadOnly))
        {
            Out(SYS_GEN | LOG_NOTICE) << "Failed to open file " << file << endl;
            return;
        }

        beginResetModel();
        QTextStream in(&fptr);
        while (!in.atEnd())
        {
            QString file = in.readLine();
            TagLib::FileRef* ref = new TagLib::FileRef(QFile::encodeName(file).data(), true, TagLib::AudioProperties::Fast);
            files.append(qMakePair(collection->find(file), ref));
        }
        endResetModel();
    }

    void PlayList::onPlaying(const kt::MediaFileRef& file)
    {
        Q_UNUSED(file);
        dataChanged(index(0, 0), index(files.count() - 1, 0));
    }

}
