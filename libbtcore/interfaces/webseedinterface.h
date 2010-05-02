/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#ifndef BTWEBSEEDINTERFACE_H
#define BTWEBSEEDINTERFACE_H

#include <kurl.h>
#include <btcore_export.h>
#include <util/constants.h>

namespace bt
{

	/**
		Interface for WebSeeds
	*/
	class BTCORE_EXPORT WebSeedInterface 
	{
	public:
		WebSeedInterface(const KUrl & url,bool user);
		virtual ~WebSeedInterface();
		
		/// Disable or enable the webseed
		virtual void setEnabled(bool on);
		
		/// Wether or not the webseed is enabled
		bool isEnabled() const {return enabled;}
		
		/// Get the URL of the webseed
		const KUrl & getUrl() const {return url;}
		
		/// Get how much data was downloaded
		Uint64 getTotalDownloaded() const {return total_downloaded;}
		
		/// Get the present status in string form
		QString getStatus() const {return status;}
		
		/// Get the current download rate in bytes per sec
		virtual Uint32 getDownloadRate() const = 0;
		
		/// Whether or not this webseed was user created
		bool isUserCreated() const {return user;}

	protected:
		KUrl url;
		Uint64 total_downloaded;
		QString status;
		bool user;
		bool enabled;
	};

}

#endif
