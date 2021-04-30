/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_PEERVIEW_HH
#define KT_PEERVIEW_HH

#include <KSharedConfig>
#include <QTreeView>

#include <interfaces/peerinterface.h>
#include <util/ptrmap.h>

class QSortFilterProxyModel;
class QMenu;

namespace kt
{
class PeerViewModel;

/**
 * View which shows a list of peers, of a torrent.
 * */
class PeerView : public QTreeView
{
    Q_OBJECT
public:
    PeerView(QWidget *parent);
    ~PeerView() override;

    /// A peer has been added
    void peerAdded(bt::PeerInterface *peer);

    /// A peer has been removed
    void peerRemoved(bt::PeerInterface *peer);

    /// Check to see if the GUI needs to be updated
    void update();

    /// Remove all items
    void removeAll();

    void saveState(KSharedConfigPtr cfg);
    void loadState(KSharedConfigPtr cfg);

private Q_SLOTS:
    void showContextMenu(const QPoint &pos);
    void banPeer();
    void kickPeer();

private:
    QMenu *context_menu;
    QSortFilterProxyModel *pm;
    PeerViewModel *model;
};
}

#endif
