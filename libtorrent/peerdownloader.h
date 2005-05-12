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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BTPEERDOWNLOADER_H
#define BTPEERDOWNLOADER_H

#include <list>
#include <qobject.h>
#include "globals.h"

namespace bt
{
	class Peer;
	class Request;
	class Piece;
	

	/**
	@author Joris Guisson
	*/
	class PeerDownloader : public QObject
	{
		Q_OBJECT
	public:
		PeerDownloader(Peer* peer);
		virtual ~PeerDownloader();

		Uint32 getNumRequests() const;
		bool isChoked() const;
		bool isNull() const {return peer == 0;}
		bool hasChunk(Uint32 idx) const;
		int grab();
		void release();
		int getNumGrabbed() const {return grabbed;}
		const Peer* getPeer() const {return peer;}
		
	public slots:
		void download(const Request & req);
		void cancel(const Request & req);
		void cancelAll();
		
	private slots:
		void piece(const Piece & p);
		void peerDestroyed();
		
	signals:
		void downloaded(const Piece & p);
		
	private:
		Peer* peer;
		std::list<Request> reqs;
		int grabbed;
	};

}

#endif
