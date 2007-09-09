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
#ifndef BTHTTPDOWNLOADER_H
#define BTHTTPDOWNLOADER_H

#include <kurl.h>
#include <util/ptrmap.h>
#include <interfaces/piecedownloader.h>

class KJob;

namespace bt
{

	/**
	 * HTTP Web Seed downloader.
	*/
	class HttpDownloader : public kt::PieceDownloader
	{
		Q_OBJECT
	public:
		HttpDownloader(const KUrl & url);
		virtual ~HttpDownloader();
		
		virtual void download(const bt::Request & req);
		virtual void cancel(const bt::Request & req);
		virtual void cancelAll();
		virtual QString getName() const;
		virtual bt::Uint32 getDownloadRate() const;
		virtual bool canAddRequest() const;
	private slots:
		void downloadJobFinished(KJob* job);
		
	private:
		KUrl url;
	};

}

#endif
