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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTUPLOADER_H
#define BTUPLOADER_H

#include <qobject.h>
#include <map>
#include "globals.h"

namespace bt
{
	class Peer;
	class ChunkManager;
	class Request;
	class PeerUploader;
	

	/**
	@author Joris Guisson
	*/
	class Uploader : public QObject
	{
		Q_OBJECT
	public:
		Uploader(ChunkManager & cman);
		virtual ~Uploader();

		Uint32 bytesUploaded() const {return uploaded;}
		Uint32 uploadRate() const;

	public slots:
		void addRequest(const Request & req);
		void cancel(const Request & req);
		void update();
		void addPeer(Peer* peer);
		void removePeer(Peer* peer);
		void removeAllPeers();
		
	private:
		ChunkManager & cman;
		Uint32 uploaded;
		std::map<const Peer*,PeerUploader*> uploaders;
		typedef std::map<const Peer*,PeerUploader*>::iterator UpItr;
		typedef std::map<const Peer*,PeerUploader*>::const_iterator UpCItr;
	};

}

#endif
