/***************************************************************************
 *   Copyright (C) 2007 by Ivan Vasić   								   *
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
#include "inactivegroup.h"

#include <klocale.h>
#include <interfaces/torrentinterface.h>

namespace kt
{
	
	InactiveGroup::InactiveGroup()
			: Group(i18n("Passive torrents"),MIXED_GROUP,"/all/passive")
	{
		setIconByName("network-disconnect");
	}
	
	
	InactiveGroup::~InactiveGroup()
	{}
	
	bool InactiveGroup::isMember(TorrentInterface * tor)
	{
		if(!tor)
			return false;
		
		const bt::TorrentStats& s = tor->getStats();
		return s.upload_rate < 100 && s.download_rate < 100;
	}

	InactiveDownloadsGroup::InactiveDownloadsGroup()
	: Group(i18n("Passive downloads"), DOWNLOADS_ONLY_GROUP,"/all/passive/downloads")
	{
		setIconByName("go-down");
	}


	InactiveDownloadsGroup::~InactiveDownloadsGroup()
	{}

	bool InactiveDownloadsGroup::isMember(TorrentInterface * tor)
	{
		if (!tor)
			return false;
	
		const bt::TorrentStats& s = tor->getStats();
	
		return (s.upload_rate < 100 && s.download_rate < 100) && !s.completed;
	}
	
	InactiveUploadsGroup::InactiveUploadsGroup()
	: Group(i18n("Passive uploads"), UPLOADS_ONLY_GROUP,"/all/passive/uploads")
	{
		setIconByName("go-up");
	}


	InactiveUploadsGroup::~InactiveUploadsGroup()
	{}

	
	bool InactiveUploadsGroup::isMember(TorrentInterface * tor)
	{
		if (!tor)
			return false;
	
		const bt::TorrentStats& s = tor->getStats();
	
		return (s.upload_rate < 100 && s.download_rate < 100) && s.completed;
	}


}