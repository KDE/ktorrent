/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTGROUPMANAGER_H
#define KTGROUPMANAGER_H

#include <QString>

#include <groups/group.h>
#include <ktcore_export.h>
#include <util/ptrmap.h>

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
    ~GroupManager() override;

    /**
     * Update the count of all groups
     * @param qman The QueueManager
     **/
    void updateCount(QueueManager *qman);

    /**
     * Find a group given it's path
     * @param path Path of the group
     * @return :Group* The Group or 0
     **/
    Group *findByPath(const QString &path);

    /**
     * Create a new user created group.
     * @param name Name of the group
     * @return Pointer to the group or nullptr, if another group already exists with the same name.
     */
    Group *newGroup(const QString &name);

    /**
     * Remove a user crated group
     * @param g The group
     */
    void removeGroup(Group *g);

    /**
     * Add a new default group.
     * @param g The group
     */
    void addDefaultGroup(Group *g);

    /**
     * Remove a default group.
     * @param g The group
     */
    void removeDefaultGroup(Group *g);

    /// Get the group off all torrents
    Group *allGroup()
    {
        return all;
    }

    typedef bt::PtrMap<QString, Group>::iterator Itr;
    typedef bt::PtrMap<QString, Group>::const_iterator CItr;

    Itr begin()
    {
        return groups.begin();
    }
    Itr end()
    {
        return groups.end();
    }

    CItr begin() const
    {
        return groups.begin();
    }
    CItr end() const
    {
        return groups.end();
    }

    /// Find  Group given a name
    Group *find(const QString &name);

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
    bool canRemove(const Group *g) const;

    /**
     * A torrent has been removed. This function checks all groups and
     * removes the torrent from it.
     * @param ti The torrent
     */
    void torrentRemoved(bt::TorrentInterface *ti);

    /**
     * Rename a group.
     * @param old_name The old name
     * @param new_name The new name
     */
    void renameGroup(const QString &old_name, const QString &new_name);

    /**
        Torrents have been loaded update all custom groups.
        @param qman The QueueManager
    */
    void torrentsLoaded(QueueManager *qman);

Q_SIGNALS:
    void groupRenamed(Group *g);
    void groupAdded(Group *g);
    void groupRemoved(Group *g);
    void customGroupChanged();

private:
    bt::PtrMap<QString, Group> groups;
    Group *all;
};

}

#endif
