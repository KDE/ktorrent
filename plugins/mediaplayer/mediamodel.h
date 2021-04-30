/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTMEDIAMODEL_H
#define KTMEDIAMODEL_H

#include "mediafile.h"
#include <QAbstractListModel>
#include <QMimeDatabase>
#include <util/constants.h>

namespace kt
{
class CoreInterface;

/**
    Interface class to find MediaFileRef objects in the collection
*/
class MediaFileCollection
{
public:
    virtual ~MediaFileCollection()
    {
    }

    /**
        Find a MediaFileRef given a path, if the path is not in the collection
        a simple file MediaFileRef will be constructed
    */
    virtual MediaFileRef find(const QString &path) = 0;
};

/**
    @author
*/
class MediaModel : public QAbstractListModel, public MediaFileCollection
{
    Q_OBJECT
public:
    MediaModel(CoreInterface *core, QObject *parent);
    ~MediaModel() override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;

    /// Get the file given a model index
    MediaFileRef fileForIndex(const QModelIndex &idx) const;

    /// Get the index of a full path
    QModelIndex indexForPath(const QString &path) const;

    MediaFileRef find(const QString &path) override;

public Q_SLOTS:
    void onTorrentAdded(bt::TorrentInterface *t);
    void onTorrentRemoved(bt::TorrentInterface *t);

private:
    CoreInterface *core;
    QList<MediaFile::Ptr> items;
    QMimeDatabase m_mimeDatabase;
};

}

#endif
