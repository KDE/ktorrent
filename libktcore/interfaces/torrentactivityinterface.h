/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TORRENTACTIVITYINTERFACE_H
#define TORRENTACTIVITYINTERFACE_H

#include <QList>
#include <interfaces/activity.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class View;
class Group;

/**
 * Small interface for classes who want to know when
 * current torrent in the gui changes.
 */
class KTCORE_EXPORT ViewListener
{
public:
    ViewListener()
    {
    }
    virtual ~ViewListener()
    {
    }

    virtual void currentTorrentChanged(bt::TorrentInterface *tc) = 0;
};

/**
    Interface for the TorrentActivity class
*/
class KTCORE_EXPORT TorrentActivityInterface : public Activity
{
public:
    TorrentActivityInterface(const QString &name, const QString &icon, QWidget *parent);
    ~TorrentActivityInterface() override;

    /// Add a view listener.
    void addViewListener(ViewListener *vl);

    /// Remove a view listener
    void removeViewListener(ViewListener *vl);

    /// Get the current torrent.
    virtual const bt::TorrentInterface *getCurrentTorrent() const = 0;

    /// Get the current torrent
    virtual bt::TorrentInterface *getCurrentTorrent() = 0;

    /// Update all actions
    virtual void updateActions() = 0;

    /// Add a tool widget to the activity
    virtual void addToolWidget(QWidget *widget, const QString &text, const QString &icon, const QString &tooltip) = 0;

    /// Remove a tool widget
    virtual void removeToolWidget(QWidget *widget) = 0;

    /// Add a new custom group
    virtual Group *addNewGroup() = 0;

protected:
    /**
     * Notifies all view listeners of the change in the current downloading TorrentInterface
     * @param tc Pointer to current TorrentInterface
     */
    void notifyViewListeners(bt::TorrentInterface *tc);

private:
    QList<ViewListener *> listeners;
};
}

#endif // TORRENTACTIVITYINTERFACE_H
