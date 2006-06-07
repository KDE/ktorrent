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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTUPLOADER_H
#define BTUPLOADER_H

#include <qobject.h>
#include "globals.h"

namespace bt
{
	class Peer;
	class PeerID;
	class ChunkManager;
	class Request;
	class PeerManager;
	

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
		Uploader(ChunkManager & cman,PeerManager & pman);
		virtual ~Uploader();

		/// Get the number of bytes uploaded.
		Uint64 bytesUploaded() const {return uploaded;}

		/// Get the upload rate of all Peers combined.
		Uint32 uploadRate() const;

		/// Set the number of bytes which have been uploaded.
		void setBytesUploaded(Uint64 b) {uploaded = b;}
	public slots:		
		/**
		 * Update every PeerUploader.
		 * @param opt_unchoked ID of optimisticly unchoked peer
		 */
		void update(Uint32 opt_unchoked);
		
	private:
		ChunkManager & cman;
		PeerManager & pman;
		Uint64 uploaded;
	};

}

#endif
