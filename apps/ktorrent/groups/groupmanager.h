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

#include <qstring.h>
#include <klocale.h>
#include <util/ptrmap.h>


namespace kt
{
	class Group;
	class TorrentInterface;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Manages all user created groups and the standard groups.
	*/
	class GroupManager : public bt::PtrMap<QString,Group> 
	{
		bt::PtrMap<QString,Group> default_groups;
		
	public:
		GroupManager();
		virtual ~GroupManager();
		
		/**
		 * Create a new user created group.
		 * @param name Name of the group
		 * @return Pointer to the group or NULL, if another group already exists with the same name.
		 */
		Group* newGroup(const QString & name);
		
		/// Get the group off all torrents
		Group* allGroup() {return findDefault(i18n("All Torrents"));}
		
		/// Get the group of downloads
		Group* downloadGroup() {return findDefault(i18n("Downloads"));}
		
		/// Get the group of seeds
		Group* uploadGroup() {return findDefault(i18n("Uploads"));}
		
		/// Get the group of queued downloads
		Group* queuedDownloadsGroup() { return findDefault(i18n("Queued downloads")); }
		
		/// Get the group of queued seeds
		Group* queuedUploadsGroup() { return findDefault(i18n("Queued uploads")); }
		
		/// Get the group of user controlled downloads
		Group* userDownloadsGroup() { return findDefault(i18n("User downloads")); }
		
		/// Get the group of user controlled seeds
		Group* userUploadsGroup() { return findDefault(i18n("User uploads")); }
		
		/// Get the group of inactive torrents
		Group* inactiveGroup() { return findDefault(i18n("Inactive torrents")); }
		
		/// Get the group of inactive downloads
		Group* inactiveDownloadsGroup() { return findDefault(i18n("Inactive downloads")); }
		
		/// Get the group of inactive uploads
		Group* inactiveUploadsGroup() { return findDefault(i18n("Inactive uploads")); }
		
		/// Get the group of inactive torrents
		Group* activeGroup() { return findDefault(i18n("Active torrents")); }
		
		/// Get the group of inactive downloads
		Group* activeDownloadsGroup() { return findDefault(i18n("Active downloads")); }
		
		/// Get the group of inactive uploads
		Group* activeUploadsGroup() { return findDefault(i18n("Active uploads")); }
		
		/// Find a default group by the given name
		Group* findDefault(const QString & name);
		
		/**
		 * Save the groups to a file.
		 * @param fn The filename
		 */
		void saveGroups(const QString & fn);
		
		/**
		 * Load the groups from a file
		 * @param fn The filename
		 */
		void loadGroups(const QString & fn);
		
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
		void torrentRemoved(TorrentInterface* ti);
		
		/**
		 * Rename a group.
		 * @param old_name The old name 
		 * @param new_name The new name
		 */
		void renameGroup(const QString & old_name,const QString & new_name); 

	};

}

#endif
