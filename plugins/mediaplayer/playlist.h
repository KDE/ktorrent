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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractItemModel>
#include <QStringList>

#include <taglib/fileref.h>
#include <util/ptrmap.h>
#include "mediafile.h"
#include "mediamodel.h"

namespace kt
{

    /**
     * PlayList containing a list of files to play.
     */
    class PlayList : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        PlayList(MediaFileCollection* collection, MediaPlayer* player, QObject* parent);
        ~PlayList();

        void addFile(const MediaFileRef& file);
        void removeFile(const MediaFileRef& file);
        MediaFileRef fileForIndex(const QModelIndex& index) const;
        void save(const QString& file);
        void load(const QString& file);
        void clear();

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& child) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        Qt::DropActions supportedDropActions() const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QStringList mimeTypes() const override;
        QMimeData* mimeData(const QModelIndexList& indexes) const override;
        bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
        bool removeRows(int row, int count, const QModelIndex& parent) override;
        bool insertRows(int row, int count, const QModelIndex& parent) override;

    private slots:
        void onPlaying(const MediaFileRef& file);

    signals:
        void itemsDropped();

    private:
        typedef QPair<MediaFileRef, TagLib::FileRef*> PlayListItem;
        mutable QList<PlayListItem> files;
        mutable QList<int> dragged_rows;
        MediaFileCollection* collection;
        MediaPlayer* player;
    };
}

#endif // PLAYLIST_H
