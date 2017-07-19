/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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

#ifndef KTGROUPMANAGER_H
#define KTGROUPMANAGER_H

#include <QString>

#include <util/ptrmap.h>
#include <ktcore_export.h>
#include <groups/group.h>

namespace bt
{
    class TorrentInterface;
}

namespace kt
{

    class QueueManager;


    /**
     * @author Joris Guisson <joris.guisson@gmail.com>
     *
     * Manages all user created groups and the standard groups.
    */
    class KTCORE_EXPORT GroupManager : public QObject
    {
        Q_OBJECT
    public:
        GroupManager();
        ~GroupManager();

        /**
         * Update the count of all groups
         * @param qman The QueueManager
         **/
        void updateCount(QueueManager* qman);

        /**
         * Find a group given it's path
         * @param path Path of the group
         * @return :Group* The Group or 0
         **/
        Group* findByPath(const QString& path);

        /**
         * Create a new user created group.
         * @param name Name of the group
         * @return Pointer to the group or nullptr, if another group already exists with the same name.
         */
        Group* newGroup(const QString& name);

        /**
         * Remove a user crated group
         * @param g The group
         */
        void removeGroup(Group* g);

        /**
         * Add a new default group.
         * @param g The group
         */
        void addDefaultGroup(Group* g);

        /**
         * Remove a default group.
         * @param g The group
         */
        void removeDefaultGroup(Group* g);

        /// Get the group off all torrents
        Group* allGroup() {return all;}

        typedef bt::PtrMap<QString, Group>::iterator Itr;

        Itr begin() {return groups.begin();}
        Itr end() {return groups.end();}

        /// Find  Group given a name
        Group* find(const QString& name);

        /// Return the custom group names
        QStringList customGroupNames();

        /**
         * Save the groups to a file.
         */
        void saveGroups();

        /**
         * Load the groups from a file
         */
        void loadGroups();

        /**
         * See if we can remove a group.
         * @param g The group
         * @return true on any user created group, false on the standard ones
         */
        bool canRemove(const Group* g) const;

        /**
         * A torrent has been removed. This function checks all groups and
         * removes the torrent from it.
         * @param ti The torrent
         */
        void torrentRemoved(bt::TorrentInterface* ti);

        /**
         * Rename a group.
         * @param old_name The old name
         * @param new_name The new name
         */
        void renameGroup(const QString& old_name, const QString& new_name);

        /**
            Torrents have been loaded update all custom groups.
            @param qman The QueueManager
        */
        void torrentsLoaded(QueueManager* qman);

    Q_SIGNALS:
        void groupRenamed(Group* g);
        void groupAdded(Group* g);
        void groupRemoved(Group* g);
        void customGroupChanged();

    private:
        bt::PtrMap<QString, Group> groups;
        Group* all;
    };

}

#endif
