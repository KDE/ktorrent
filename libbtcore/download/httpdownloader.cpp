/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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
#include "httpdownloader.h"
#include <math.h>
#include <torrent/torrent.h>
#include "httpconnection.h"

namespace bt
{

	HttpDownloader::HttpDownloader(const KUrl & url,const Torrent & tor) : url(url),tor(tor),conn(0)
	{}


	HttpDownloader::~HttpDownloader()
	{
		delete conn;
	}

	void HttpDownloader::download(const bt::Request & req)
	{
	}
	
	void HttpDownloader::cancel(const bt::Request & req)
	{
	}
	
	void HttpDownloader::cancelAll()
	{
		delete conn;
		conn = 0;
	}
	
	QString HttpDownloader::getName() const
	{
		return url.url();
	}
	
	bt::Uint32 HttpDownloader::getDownloadRate() const
	{
		return (Uint32)ceil(conn->getDownloadRate());
	}
	
	bool HttpDownloader::canAddRequest() const
	{
		return false;
	}
}

#include "httpdownloader.moc"
