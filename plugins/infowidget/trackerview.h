/*
    SPDX-FileCopyrightText: 2006-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2006-2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRACKERVIEW_H
#define TRACKERVIEW_H

#include "ui_trackerview.h"

#include <KSharedConfig>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <interfaces/torrentinterface.h>

namespace kt
{
class TrackerModel;

/**
 * @author Ivan Vasic <ivan@ktorrent.org>
 */
class TrackerView : public QWidget, public Ui_TrackerView
{
    Q_OBJECT
public:
    TrackerView(QWidget *parent);
    ~TrackerView() override;

    void update();
    void changeTC(bt::TorrentInterface *ti);
    void saveState(KSharedConfigPtr cfg);
    void loadState(KSharedConfigPtr cfg);

public Q_SLOTS:
    void updateClicked();
    void restoreClicked();
    void changeClicked();
    void removeClicked();
    void addClicked();
    void scrapeClicked();
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void torrentChanged(bt::TorrentInterface *ti);
    bt::TrackerInterface *selectedTracker() const;

private:
    bt::TorrentInterface::WPtr tc;
    TrackerModel *model;
    QSortFilterProxyModel *proxy_model;
    QStringList tracker_hints;
    bool header_state_loaded;
    QMenu *m_ContextMenu;
};
}
#endif
