/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTWEBSEEDSTAB_H
#define KTWEBSEEDSTAB_H

#include <QSortFilterProxyModel>
#include <QWidget>

#include <KConfigGroup>
#include <KSharedConfig>

#include "ui_webseedstab.h"
#include <interfaces/torrentinterface.h>

namespace kt
{
class WebSeedsModel;

/**
    Tab which displays the list of webseeds of a torrent, and allows you to add or remove them.
*/
class WebSeedsTab : public QWidget, public Ui_WebSeedsTab
{
    Q_OBJECT
public:
    WebSeedsTab(QWidget *parent);
    ~WebSeedsTab() override;

    /**
     * Switch to a different torrent.
     * @param tc The torrent
     */
    void changeTC(bt::TorrentInterface *tc);

    /// Check to see if the GUI needs to be updated
    void update();

    void saveState(KSharedConfigPtr cfg);
    void loadState(KSharedConfigPtr cfg);

private Q_SLOTS:
    void addWebSeed();
    void removeWebSeed();
    void disableAll();
    void enableAll();
    void onWebSeedTextChanged(const QString &ws);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void selectionChanged(const QModelIndexList &indexes);

private:
    bt::TorrentInterface::WPtr curr_tc;
    WebSeedsModel *model;
    QSortFilterProxyModel *proxy_model;
};

}

#endif
