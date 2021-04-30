/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTPEERVIEWMODEL_H
#define KTPEERVIEWMODEL_H

#include <QAbstractTableModel>
#include <QIcon>
#include <QVector>

#include <interfaces/peerinterface.h>

namespace kt
{
class GeoIPManager;

/**
    @author Joris Guisson
    Model for the PeerView
*/
class PeerViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    PeerViewModel(QObject *parent);
    ~PeerViewModel() override;

    /// A peer has been added
    void peerAdded(bt::PeerInterface *peer);

    /// A peer has been removed
    void peerRemoved(bt::PeerInterface *peer);

    /**
     * Update the model
     */
    void update();

    /**
        Clear the model
    */
    void clear();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    bt::PeerInterface *indexToPeer(const QModelIndex &idx);

public:
    struct Item {
        bt::PeerInterface *peer;
        mutable bt::PeerInterface::Stats stats;
        QString country;
        QIcon flag;

        Item(bt::PeerInterface *peer, GeoIPManager *geo_ip);

        bool changed() const;
        QVariant data(int col) const;
        QVariant decoration(int col) const;
        QVariant sortData(int col) const;
    };

private:
    QVector<Item *> items;
    GeoIPManager *geo_ip;
};

}

#endif
