/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTQUEUEMANAGERMODEL_H
#define KTQUEUEMANAGERMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include <torrent/queuemanager.h>

class QMimeData;

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class QueueManager;

/**
 * @author Joris Guisson
 *
 * Model for the QM
 */
class QueueManagerModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    QueueManagerModel(QueueManager *qman, QObject *parent);
    ~QueueManagerModel() override;

    void setShowUploads(bool on);
    void setShowDownloads(bool on);
    void setShowNotQueued(bool on);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    /**
     * Move items one row up
     * @param row The row of the item
     * @param count The number of items to move
     */
    void moveUp(int row, int count);

    /**
     * Move items one row down
     * @param row The row of the item
     * @param count The number of items to move
     */
    void moveDown(int row, int count);

    /**
     * Move items to the top
     * @param row The row of the item
     * @param count The number of items to move
     */
    void moveTop(int row, int count);

    /**
     * Move items to the bottom
     * @param row The row of the item
     * @param count The number of items to move
     */
    void moveBottom(int row, int count);

    /**
     * Update the model
     */
    void update();

    /**
        Given a search text find a matching torrent
    */
    QModelIndex find(const QString &text);

public Q_SLOTS:
    void onTorrentAdded(bt::TorrentInterface *tc);
    void onTorrentRemoved(bt::TorrentInterface *tc);
    void onQueueOrdered();
    void onTorrentStatusChanged(bt::TorrentInterface *tc);

private:
    struct Item {
        bt::TorrentInterface *tc;
        bt::Int64 stalled_time;

        bool operator<(const Item &item) const
        {
            return tc->getPriority() < item.tc->getPriority();
        }
    };

    bool visible(const bt::TorrentInterface *tc);
    void updateQueue();
    void swapItems(int a, int b);
    void dumpQueue();
    void updatePriorities();
    void softReset();

private:
    QueueManager *qman;
    QList<Item> queue;
    mutable QList<int> dragged_items;
    QString search_text;

    bool show_uploads;
    bool show_downloads;
    bool show_not_queud;
};

}

#endif
