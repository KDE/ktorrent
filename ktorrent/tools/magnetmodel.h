/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_MAGNETMODEL_H
#define KT_MAGNETMODEL_H

#include <QAbstractTableModel>
#include <QPointer>
#include <util/constants.h>

namespace bt
{
class MagnetDownloader;
}

namespace kt
{
class MagnetManager;

class MagnetModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    MagnetModel(MagnetManager *magnetManager, QObject *parent = nullptr);
    ~MagnetModel() override;

    /// Remove a magnet downloader
    void removeMagnets(int row, int count);

    /// Start a magnet downloader
    void start(int row, int count);

    /// Stop a magnet downloader
    void stop(int row, int count);

    /// Check if the magnet downloader that correspond to row is stopped
    bool isStopped(int row) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;

public Q_SLOTS:
    void onUpdateQueue(bt::Uint32 idx, bt::Uint32 count);

private:
    QString displayName(const bt::MagnetDownloader *md) const;
    QString status(int row) const;

private:
    int currentRows;
    QPointer<MagnetManager> mman;
};

}

#endif // KT_MAGNETMODEL_H
