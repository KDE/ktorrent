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
#include "activegroup.h"

#include <klocale.h>
#include <interfaces/torrentinterface.h>

namespace kt
{
	
	ActiveGroup::ActiveGroup()
			: Group(i18n("Active torrents"),MIXED_GROUP,"/all/active")
	{
		setIconByName("network-connect");
	}
	
	
	ActiveGroup::~ActiveGroup()
	{}
	
	bool ActiveGroup::isMember(TorrentInterface * tor)
	{
		if(!tor)
			return false;
		
		const bt::TorrentStats& s = tor->getStats();
		return s.upload_rate >= 100 || s.download_rate >= 100;
	}


	ActiveUploadsGroup::ActiveUploadsGroup()
	: Group(i18n("Active uploads"), UPLOADS_ONLY_GROUP,"/all/active/uploads")
	{
		setIconByName("go-up");
	}


	ActiveUploadsGroup::~ActiveUploadsGroup()
	{}

	bool ActiveUploadsGroup::isMember(TorrentInterface * tor)
	{
		if (!tor)
			return false;
	
		const bt::TorrentStats& s = tor->getStats();
	
		return (s.upload_rate >= 100 || s.download_rate >= 100) && s.completed;
	}

	
	ActiveDownloadsGroup::ActiveDownloadsGroup()
	: Group(i18n("Active downloads"),DOWNLOADS_ONLY_GROUP,"/all/active/downloads")
	{
		setIconByName("go-down");
	}


	ActiveDownloadsGroup::~ActiveDownloadsGroup()
	{}

	bool ActiveDownloadsGroup::isMember(TorrentInterface * tor)
	{
		if (!tor)
			return false;

		const bt::TorrentStats& s = tor->getStats();

		return (s.upload_rate >= 100 || s.download_rate >= 100) && !s.completed;
	}
}
