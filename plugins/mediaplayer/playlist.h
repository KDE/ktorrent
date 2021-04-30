/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractItemModel>
#include <QStringList>

#include "mediafile.h"
#include "mediamodel.h"
#include <taglib/fileref.h>
#include <util/ptrmap.h>

namespace kt
{
/**
 * PlayList containing a list of files to play.
 */
class PlayList : public QAbstractItemModel
{
    Q_OBJECT
public:
    PlayList(MediaFileCollection *collection, MediaPlayer *player, QObject *parent);
    ~PlayList() override;

    void addFile(const MediaFileRef &file);
    void removeFile(const MediaFileRef &file);
    MediaFileRef fileForIndex(const QModelIndex &index) const;
    void save(const QString &file);
    void load(const QString &file);
    void clear();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;

private Q_SLOTS:
    void onPlaying(const MediaFileRef &file);

Q_SIGNALS:
    void itemsDropped();

private:
    typedef QPair<MediaFileRef, TagLib::FileRef *> PlayListItem;
    mutable QList<PlayListItem> files;
    mutable QList<int> dragged_rows;
    MediaFileCollection *collection;
    MediaPlayer *player;
};
}

#endif // PLAYLIST_H
