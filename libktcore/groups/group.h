/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
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

#ifndef KTGROUP_H
#define KTGROUP_H

#include <QIcon>
#include <QString>

#include <ktcore_export.h>
#include <util/constants.h>

namespace bt
{
    class BEncoder;
    class BDictNode;
    class TorrentInterface;
}

namespace kt
{

    class QueueManager;
    using bt::TorrentInterface;

    /**
     * @author Joris Guisson <joris.guisson@gmail.com>
     *
     * Base class for all groups. Subclasses should only implement the
     * isMember function, but can also provide save and load
     * functionality.
     */
    class KTCORE_EXPORT Group : public QObject
    {
        Q_OBJECT
    public:
        enum Properties
        {
            UPLOADS_ONLY_GROUP = 1,
            DOWNLOADS_ONLY_GROUP = 2,
            MIXED_GROUP = 3,
            CUSTOM_GROUP = 4
        };

        struct KTCORE_EXPORT Policy
        {
            QString default_save_location;
            QString default_move_on_completion_location;
            float max_share_ratio;
            float max_seed_time;
            bt::Uint32 max_upload_rate;
            bt::Uint32 max_download_rate;
            bool only_apply_on_new_torrents;

            Policy();
        };

        /**
         * Create a new group.
         * @param name The name of the group
         * @param flags Properties of the group
         * @param path Path in the group tree (e.g /all/downloads/foo, last item in path should be the groups internal name)
         */
        Group(const QString& name, int flags, const QString& path);
        virtual ~Group();

        /// See if this is a standard group.
        bool isStandardGroup() const {return !(flags & CUSTOM_GROUP);}

        /// Get the group flags
        int groupFlags() const {return flags;}

        /**
         * Rename the group.
         * @param nn The new name
         */
        void rename(const QString& nn);

        /**
         * Set the group icon by name.
         * @param in The icon name
         */
        void setIconByName(const QString& in);

        /// Get the name of the group
        const QString& groupName() const {return name;}

        /// Get the icon of the group
        const QIcon& groupIcon() const {return icon;}

        /// Name of the group icon
        const QString& groupIconName() const {return icon_name;}

        /// Get the group policy
        const Policy& groupPolicy() const {return policy;}

        /// Path in the group tree
        const QString& groupPath() const {return path;}

        /// Get the number of running torrents
        int runningTorrents() const {return running;}

        /// Total torrents
        int totalTorrents() const {return total;}

        /// Set the group policy
        void setGroupPolicy(const Policy& p);

        /**
         * Save the torrents.The torrents should be save in a bencoded file.
         * @param enc The BEncoder
         */
        virtual void save(bt::BEncoder* enc);


        /**
         * Load the torrents of the group from a BDictNode.
         * @param n The BDictNode
         */
        virtual void load(bt::BDictNode* n);


        /**
         * Test if a torrent is a member of this group.
         * @param tor The torrent
         */
        virtual bool isMember(TorrentInterface* tor) = 0;

        /**
         * The torrent has been removed and is about to be deleted.
         * Subclasses should make sure that they don't have dangling
         * pointers to this torrent.
         * @param tor The torrent
         */
        virtual void torrentRemoved(TorrentInterface* tor);

        /**
         * Subclasses should implement this, if they want to have torrents added to them.
         * @param tor The torrent
         * @param new_torrent Indicates whether this is a newly created or opened torrent
         */
        virtual void addTorrent(TorrentInterface* tor, bool new_torrent);

        /**
         * Subclasses should implement this, if they want to have torrents removed from them.
         * @param tor The torrent
         */
        virtual void removeTorrent(TorrentInterface* tor);

        /**
         * Called when the policy has been changed.
         */
        virtual void policyChanged();

        /**
         * Update the running and total count
         * @param qman The QueueManager
         **/
        void updateCount(QueueManager* qman);

    protected:
        QString name;
        QIcon icon;
        QString icon_name;
        int flags;
        Policy policy;
        QString path;
        int running;
        int total;
    };

}

#endif
