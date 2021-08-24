/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KTDOWNLOADORDERPLUGIN_H
#define KTDOWNLOADORDERPLUGIN_H

#include <interfaces/plugin.h>
#include <interfaces/torrentactivityinterface.h>
#include <util/ptrmap.h>

namespace kt
{
class DownloadOrderManager;

/**
    @author
*/
class DownloadOrderPlugin : public Plugin, public ViewListener
{
    Q_OBJECT
public:
    DownloadOrderPlugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~DownloadOrderPlugin() override;

    void load() override;
    void unload() override;
    void currentTorrentChanged(bt::TorrentInterface *tc) override;
    QString parentPart() const override
    {
        return QStringLiteral("torrentactivity");
    }

    /// Get the download order manager for a torrent (returns 0 if none exists)
    DownloadOrderManager *manager(bt::TorrentInterface *tc);

    /// Create a manager for a torrent
    DownloadOrderManager *createManager(bt::TorrentInterface *tc);

    /// Destroy a manager
    void destroyManager(bt::TorrentInterface *tc);

private Q_SLOTS:
    void showDownloadOrderDialog();
    void torrentAdded(bt::TorrentInterface *tc);
    void torrentRemoved(bt::TorrentInterface *tc);

private:
    QAction *download_order_action;
    bt::PtrMap<bt::TorrentInterface *, DownloadOrderManager> managers;
};

}

#endif
