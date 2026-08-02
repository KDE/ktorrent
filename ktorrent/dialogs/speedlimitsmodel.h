/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSPEEDLIMITSMODEL_H
#define KTSPEEDLIMITSMODEL_H

#include <QAbstractTableModel>

#include <util/constants.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class Core;

/**
 * Model for the SpeedLimitsDlg main list view
 */
class SpeedLimitsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SpeedLimitsModel(Core *core, QObject *parent);
    ~SpeedLimitsModel() override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void apply();

    enum class Column : int {
        TORRENT,
        DOWNLOAD_LIMIT,
        UPLOAD_LIMIT,
        ASSURED_DOWNLOAD_SPEED,
        ASSURED_UPLOAD_SPEED,
        _NUMBER_OF_COLUMNS,
    };
    static constexpr auto NUM_COLUMNS = static_cast<int>(Column::_NUMBER_OF_COLUMNS);

Q_SIGNALS:
    void enableApply(bool on);

private:
    bt::TorrentInterface *torrentForIndex(const QModelIndex &index) const;

private Q_SLOTS:
    void onTorrentAdded(bt::TorrentInterface *tc);
    void onTorrentRemoved(bt::TorrentInterface *tc);

private:
    struct Limits {
        bt::Uint32 up;
        bt::Uint32 up_original;
        bt::Uint32 down;
        bt::Uint32 down_original;
        bt::Uint32 assured_up;
        bt::Uint32 assured_up_original;
        bt::Uint32 assured_down;
        bt::Uint32 assured_down_original;
    };

    Core *core;
    QMap<bt::TorrentInterface *, Limits> limits;
};

}

#endif
