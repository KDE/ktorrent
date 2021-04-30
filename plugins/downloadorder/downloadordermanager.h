/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTDOWNLOADORDERMANAGER_H
#define KTDOWNLOADORDERMANAGER_H

#include <QList>
#include <QObject>
#include <util/constants.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
/**
    Class which manages the file download order for a single torrent
*/
class DownloadOrderManager : public QObject
{
    Q_OBJECT
public:
    DownloadOrderManager(bt::TorrentInterface *tor);
    ~DownloadOrderManager() override;

    /// See if the file download order is enabled
    bool enabled() const
    {
        return order.count() > 0;
    }

    /// Save the order from torX/download_order
    void save();

    /// Load the order to torX/download_order
    void load();

    /// Enable the download order
    void enable();

    /// Disable the download order
    void disable();

    typedef QList<bt::Uint32> Order;

    /// Get the download order
    const Order &downloadOrder() const
    {
        return order;
    }

    /// Set the order
    void setDownloadOrder(const Order &norder)
    {
        order = norder;
    }

public Q_SLOTS:
    /**
     * Change file priorities if needed
     */
    void update();

    /**
     * Change file priorities if needed
     */
    void chunkDownloaded(bt::TorrentInterface *me, bt::Uint32 chunk);

private:
    bt::Uint32 nextIncompleteFile();

private:
    bt::TorrentInterface *tor;
    QList<bt::Uint32> order;
    bt::Uint32 current_high_priority_file;
    bt::Uint32 current_normal_priority_file;
};

}

#endif
