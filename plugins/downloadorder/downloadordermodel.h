/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTDOWNLOADORDERMODEL_H
#define KTDOWNLOADORDERMODEL_H

#include <QAbstractListModel>
#include <util/constants.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
/**
    Model for the download order in the dialog
*/
class DownloadOrderModel : public QAbstractListModel
{
    Q_OBJECT
public:
    DownloadOrderModel(bt::TorrentInterface *tor, QObject *parent);
    ~DownloadOrderModel() override;

    /// Initialize the order
    void initOrder(const QList<bt::Uint32> &sl)
    {
        order = sl;
    }

    /// Get the order
    const QList<bt::Uint32> &downloadOrder() const
    {
        return order;
    }

    /// Find a text in the file list
    QModelIndex find(const QString &text);

    /// Clear high lights
    void clearHighLights();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    void moveUp(int row, int count);
    void moveDown(int row, int count);
    void moveTop(int row, int count);
    void moveBottom(int row, int count);

public Q_SLOTS:
    void sortByName();
    void sortBySeasonsAndEpisodes();
    void sortByAlbumTrackOrder();

private:
    bt::TorrentInterface *tor;
    QList<bt::Uint32> order;
    QString current_search_text;
};

}

#endif
