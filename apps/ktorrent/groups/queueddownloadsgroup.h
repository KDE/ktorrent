/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#ifndef KTQUEUEDDOWNLOADSGROUP_H
#define KTQUEUEDDOWNLOADSGROUP_H

#include <group.h>

namespace kt
{
	class TorrentInterface;

	/**
	 * Torrents that are queued and in downloading phase.
	 * @author Ivan Vasic <ivasic@gmail.com>
	*/
	class QueuedDownloadsGroup : public Group
	{
		public:
			QueuedDownloadsGroup();
			virtual ~QueuedDownloadsGroup();
			
			virtual bool isMember(TorrentInterface* tor);
	};

}

#endif
