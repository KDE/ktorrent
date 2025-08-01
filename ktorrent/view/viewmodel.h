/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTVIEWMODEL_H
#define KTVIEWMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include <torrent/torrentstats.h>
#include <util/constants.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class View;
class ViewDelegate;
class Core;
class Group;

/**
 * @author Joris Guisson
 *
 * Model for the main torrent view
 */
class ViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ViewModel(Core *core, View *parent);
    ~ViewModel() override;

    /**
     * Set the Group to filter
     * @param g The group
     */
    void setGroup(Group *g);

    /**
     * Update the model, checks if data has changed.
     $ @param delegate The ViewDelegate, so we don't hide extended items
     * @param force_resort Force a resort
     * @return true if the model got resorted
     */
    bool update(ViewDelegate *delegate, bool force_resort = false);

    /**
     * Set the current filter string
     * @param filter The filter string
     */
    void setFilterString(const QString &filter);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;

    /**
     * Emit the data changed signal
     * @param row The row
     * @param col The column
     */
    void emitDataChanged(int row, int col);

    /**
     * Get a list of torrents from an index list.
     * @param idx The index list
     * @param tlist The torrent list to fill
     */
    void torrentsFromIndexList(const QModelIndexList &idx, QList<bt::TorrentInterface *> &tlist);

    /**
     * Get a torrent from a model index.
     * @param index The model index
     * @return The torrent if the index is valid and in the proper range, 0 otherwise
     */
    bt::TorrentInterface *torrentFromIndex(const QModelIndex &index) const;

    /**
     * Get a torrent from a row.
     * @param index The row index
     * @return The torrent if the index is valid and in the proper range, 0 otherwise
     */
    bt::TorrentInterface *torrentFromRow(int index) const;

    /**
     * Get all torrents
     * @param tlist The list of torrents to fill
     */
    void allTorrents(QList<bt::TorrentInterface *> &tlist) const;

    /**
     * Visit all visible torrents in the model, and apply an action to them
     */
    template<class Action>
    void visit(Action &a)
    {
        for (Item *item : std::as_const(torrents)) {
            if (item->visible(group, filter_string))
                if (!a(item->tc))
                    break;
        }
    }

    /// Get the list of indexes which need to be updated
    const QModelIndexList &updateList() const
    {
        return update_list;
    }

public Q_SLOTS:
    void addTorrent(bt::TorrentInterface *ti);
    void removeTorrent(bt::TorrentInterface *ti);
    void sort(int col, Qt::SortOrder order) override;
    void onExit();

Q_SIGNALS:
    void sorted();

public:
    enum Column {
        NAME = 0,
        BYTES_DOWNLOADED,
        SESSION_BYTES_DOWNLOADED,
        TOTAL_BYTES_TO_DOWNLOAD,
        BYTES_LEFT,
        BYTES_UPLOADED,
        SESSION_BYTES_UPLOADED,
        DOWNLOAD_RATE,
        UPLOAD_RATE,
        ETA,
        SEEDERS,
        LEECHERS,
        PERCENTAGE,
        SHARE_RATIO,
        DOWNLOAD_TIME,
        SEED_TIME,
        DOWNLOAD_LOCATION,
        TIME_ADDED,
        LAST_ACTIVITY,
        _NUMBER_OF_COLUMNS,
    };

    struct Item {
        bt::TorrentInterface *tc;
        // cached values to avoid unneeded updates
        bt::TorrentStatus status;
        bt::Uint64 bytes_downloaded;
        bt::Uint64 session_bytes_downloaded;
        bt::Uint64 bytes_uploaded;
        bt::Uint64 session_bytes_uploaded;
        bt::Uint64 total_bytes_to_download;
        bt::Uint64 bytes_left;
        bt::Uint32 download_rate;
        bt::Uint32 upload_rate;
        bt::Uint32 seeders_total;
        bt::Uint32 seeders_connected_to;
        bt::Uint32 leechers_total;
        bt::Uint32 leechers_connected_to;
        double percentage;
        float share_ratio;
        bt::Uint32 runtime_dl;
        bt::Uint32 runtime_ul;
        int eta;
        bool hidden;
        QDateTime time_added;
        bool highlight;
        bt::TimeStamp last_activity;

        Item(bt::TorrentInterface *tc);

        bool update(int row, int sort_column, QModelIndexList &to_update, ViewModel *model);
        QVariant data(int col) const;
        QVariant color(int col) const;
        QVariant statusIcon() const;
        bool lessThan(int col, const Item *other) const;
        bool visible(Group *group, const QString &filter_string) const;
    };

private:
    Core *core;
    View *view;
    QList<Item *> torrents;
    int sort_column;
    Qt::SortOrder sort_order;
    Group *group;
    int num_visible;
    QModelIndexList update_list;
    QString filter_string;
};

}

#endif
