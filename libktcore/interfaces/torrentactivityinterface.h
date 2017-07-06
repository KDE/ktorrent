/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

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
        ViewListener() {}
        virtual ~ViewListener() {}

        virtual void currentTorrentChanged(bt::TorrentInterface* tc) = 0;
    };


    /**
        Interface for the TorrentActivity class
    */
    class KTCORE_EXPORT TorrentActivityInterface : public Activity
    {
    public:
        TorrentActivityInterface(const QString& name, const QString& icon, QWidget* parent);
        ~TorrentActivityInterface();

        /// Add a view listener.
        void addViewListener(ViewListener* vl);

        /// Remove a view listener
        void removeViewListener(ViewListener* vl);

        /// Get the current torrent.
        virtual const bt::TorrentInterface* getCurrentTorrent() const = 0;

        /// Get the current torrent
        virtual bt::TorrentInterface* getCurrentTorrent() = 0;

        /// Update all actions
        virtual void updateActions() = 0;

        /// Add a tool widget to the activity
        virtual void addToolWidget(QWidget* widget, const QString& text, const QString& icon, const QString& tooltip) = 0;

        /// Remove a tool widget
        virtual void removeToolWidget(QWidget* widget) = 0;

        /// Add a new custom group
        virtual Group* addNewGroup() = 0;

    protected:
        /**
        * Notifies all view listeners of the change in the current downloading TorrentInterface
        * @param tc Pointer to current TorrentInterface
        */
        void notifyViewListeners(bt::TorrentInterface* tc);

    private:
        QList<ViewListener*> listeners;
    };
}

#endif // TORRENTACTIVITYINTERFACE_H
