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
#include <ktcore_export.h>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{
	class Group;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Manages all user created groups and the standard groups.
	*/
	class KTCORE_EXPORT GroupManager : public QObject, public bt::PtrMap<QString,Group>
	{
		Q_OBJECT
		
		Group* all;
		Group* download;
		Group* upload;
		Group* queued_downloads;
		Group* queued_uploads;
		Group* user_downloads;
		Group* user_uploads;
		Group* inactive;
		Group* inactive_downloads;
		Group* inactive_uploads;
		Group* active;
		Group* active_downloads;
		Group* active_uploads;
		Group* ungrouped;
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
		Group* allGroup() {return all;}
		
		/// Get the group of downloads
		Group* downloadGroup() {return download;}
		
		/// Get the group of seeds
		Group* uploadGroup() {return upload;}
		
		/// Get the group of queued downloads
		Group* queuedDownloadsGroup() { return queued_downloads; }
		
		/// Get the group of queued seeds
		Group* queuedUploadsGroup() { return queued_uploads; }
		
		/// Get the group of user controlled downloads
		Group* userDownloadsGroup() { return user_downloads; }
		
		/// Get the group of user controlled seeds
		Group* userUploadsGroup() { return user_uploads; }
		
		/// Get the group of inactive torrents
		Group* inactiveGroup() { return inactive; }
		
		/// Get the group of inactive downloads
		Group* inactiveDownloadsGroup() { return inactive_downloads; }
		
		/// Get the group of inactive uploads
		Group* inactiveUploadsGroup() { return inactive_uploads; }
		
		/// Get the group of inactive torrents
		Group* activeGroup() { return active; }
		
		/// Get the group of inactive downloads
		Group* activeDownloadsGroup() { return active_downloads; }
		
		/// Get the group of inactive uploads
		Group* activeUploadsGroup() { return active_uploads; }
		
		/// Get the ungrouped Group
		Group* ungroupedGroup() { return ungrouped;}
		
		/// Find a default Group given a name
		Group* findDefault(const QString & name);
		
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
		
		virtual bool erase(const QString & key);

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
		void renameGroup(const QString & old_name,const QString & new_name); 
	
	signals:
		void customGroupsChanged(QString oldName=QString(), QString newName=QString());
		

	};

}

#endif
