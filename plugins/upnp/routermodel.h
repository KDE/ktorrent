/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTROUTERMODEL_H
#define KTROUTERMODEL_H

#include <QAbstractTableModel>

namespace bt
{
class UPnPRouter;
class WaitJob;
}

namespace net
{
struct Port;
}

namespace kt
{
/**
    Model for all the detected UPnP routers.
*/
class RouterModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RouterModel(QObject *parent);
    ~RouterModel() override;

    /// Add a router tot the model
    void addRouter(bt::UPnPRouter *r);

    /// Get a router given an index
    bt::UPnPRouter *routerForIndex(const QModelIndex &index);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;

    void update();

    /// Forward a ports on all routers
    void forward(const net::Port &port);

    /// Undo forward a ports on all routers
    void undoForward(const net::Port &port, bt::WaitJob *wjob);

private:
    QString ports(const bt::UPnPRouter *r) const;

private:
    QList<bt::UPnPRouter *> routers;
};

}

#endif
