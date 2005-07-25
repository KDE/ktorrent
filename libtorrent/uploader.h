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
	 * @author Joris Guisson
	 *
	 * Class which manages the uploading of data. It has a PeerUploader for
	 * each Peer.
	 */
	class Uploader : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Constructor, sets the ChunkManager. 
		 * @param cman The ChunkManager
		 */
		Uploader(ChunkManager & cman);
		virtual ~Uploader();

		/// Get the number of bytes uploaded.
		Uint32 bytesUploaded() const {return uploaded;}

		/// Get the upload rate of all Peers combined.
		Uint32 uploadRate() const;

		/// Set the number of bytes which have been uploaded.
		void setBytesUploaded(Uint32 b) {uploaded = b;}
	public slots:
		/**
		 * Add a Request for a piece of a Chunk.
		 * @param req The request
		 */
		void addRequest(const Request & req);
		
		/**
		 * Cancel a Request.
		 * @param req The request
		 */
		void cancel(const Request & req);
		
		/**
		 * Update every PeerUploader.
		 */
		void update();
		
		/**
		 * Add a Peer, this will create a PeerUploader for the Peer.
		 * @param peer The Peer
		 */
		void addPeer(Peer* peer);

		/**
		 * Remove a Peer, this will get rid of the Peer's PeerUploader.
		 * @param peer The Peer
		 */
		void removePeer(Peer* peer);

		/**
		 * Remove all Peer's and PeerUploader's.
		 */
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
