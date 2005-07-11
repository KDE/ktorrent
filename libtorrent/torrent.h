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
#ifndef BTTORRENT_H
#define BTTORRENT_H

#include <kurl.h>
#include <qvaluevector.h>
#include "sha1hash.h"
#include "globals.h"
#include "peerid.h"

namespace bt
{
	class BNode;
	class BValueNode;
	class BDictNode;
	class BListNode;
	class AnnounceList;
	
	/**
	 * @author Joris Guisson
	 * @brief Loads a .torrent file
	 * 
	 * Loads a torrent file and calculates some miscelanious other data,
	 * like the info_hash and the peer_id.
	 */
	class Torrent
	{
	public:
		struct File
		{
			QString path;
			Uint32 size;
		};
	public:
		Torrent();
		virtual ~Torrent();

		/**
		 * Load a .torrent file.
		 * @param file The file
		 * @throw Error if something goes wrong
		 */
		void load(const QString & file);
		
		void debugPrintInfo();
		
		/// Get the number of chunks.
		unsigned int getNumChunks() const {return hash_pieces.size();}
		
		/// Get the size of a chunk.
		unsigned int getChunkSize() const {return piece_length;}
		
		/// Get the tracker URL.
		KURL getTrackerURL(bool last_was_succesfull = true) const;
		
		/// Get the info_hash.
		const SHA1Hash & getInfoHash() const {return info_hash;}
		
		/// Get out peer_id.
		const PeerID & getPeerID() const {return peer_id;}
		
		/// Get the file size in number of bytes.
		unsigned int getFileLength() const {return file_length;}
		
		/// Get the suggested name.
		QString getNameSuggestion() const {return name_suggestion;}
		
		/**
		 * Verify wether a hash matches the hash
		 * of a Chunk
		 * @param h The hash
		 * @param index The index of the chunk
		 * @return true if they match
		 */
		bool verifyHash(const SHA1Hash & h,Uint32 index);

		/// Get the number of tracker URL's
		unsigned int getNumTrackerURLs() const;
		
		const SHA1Hash & getHash(Uint32 idx) const;

		bool isMultiFile() const {return files.count() > 0;}
		Uint32 getNumFiles() const {return files.count();}
		void getFile(Uint32 idx,Torrent::File & file);
	private:
		void loadInfo(BDictNode* node);
		void loadTrackerURL(BValueNode* node);
		void loadPieceLength(BValueNode* node);
		void loadFileLength(BValueNode* node);
		void loadHash(BValueNode* node);
		void loadName(BValueNode* node);
		void loadFiles(BListNode* node);
		void loadAnnounceList(BNode* node);
		
	private:
		KURL tracker_url;
		QString name_suggestion;
		unsigned int piece_length;
		unsigned int file_length;
		SHA1Hash info_hash;
		PeerID peer_id;
		QValueVector<SHA1Hash> hash_pieces;
		QValueVector<File> files;
		AnnounceList* anon_list;
		QString encoding;
	};

}

#endif
