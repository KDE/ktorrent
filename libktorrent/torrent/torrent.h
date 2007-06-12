/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
#ifndef BTTORRENT_H
#define BTTORRENT_H

#include <kurl.h>
#include <qvaluevector.h>
#include <util/sha1hash.h>
#include <util/constants.h>
#include <interfaces/torrentinterface.h>
#include "globals.h"
#include "peerid.h"
#include "torrentfile.h"



namespace bt
{
	class BNode;
	class BValueNode;
	class BDictNode;
	class BListNode;


	struct TrackerTier
	{
		KURL::List urls;
		TrackerTier* next;
		
		TrackerTier() : next(0)
		{}
		
		~TrackerTier() 
		{
			delete next;
		}
	};
	
	
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
		Torrent();
		virtual ~Torrent();

		/**
		 * Load a .torrent file.
		 * @param file The file
		 * @param verbose Wether to print information to the log
		 * @throw Error if something goes wrong
		 */
		void load(const QString & file,bool verbose);
		
		/**
		 * Load a .torrent file.
		 * @param data The data
		 * @param verbose Wether to print information to the log
		 * @throw Error if something goes wrong
		 */
		void load(const QByteArray & data,bool verbose);
		
		void debugPrintInfo();
		
		/// Get the number of chunks.
		Uint32 getNumChunks() const {return hash_pieces.size();}
		
		/// Get the size of a chunk.
		Uint64 getChunkSize() const {return piece_length;}
		
		/// Get the info_hash.
		const SHA1Hash & getInfoHash() const {return info_hash;}
		
		/// Get our peer_id.
		const PeerID & getPeerID() const {return peer_id;}
		
		/// Get the file size in number of bytes.
		Uint64 getFileLength() const {return file_length;}
		
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

		/**
		 * Get the hash of a Chunk. Throws an Error
		 * if idx is out of bounds.
		 * @param idx Index of Chunk
		 * @return The SHA1 hash of the chunk
		 */
		const SHA1Hash & getHash(Uint32 idx) const;

		/// See if we have a multi file torrent.
		bool isMultiFile() const {return files.count() > 0;}

		/// Get the number of files in a multi file torrent.
		/// If we have a single file torrent, this will return 0.
		Uint32 getNumFiles() const {return files.count();}
		
		/**
		 * Get a TorrentFile. If the index is out of range, or
		 * we have a single file torrent we return a null TorrentFile.
		 * @param idx Index of the file
		 * @param A reference to the file
		 */
		TorrentFile & getFile(Uint32 idx);

		/**
		 * Get a TorrentFile. If the index is out of range, or
		 * we have a single file torrent we return a null TorrentFile.
		 * @param idx Index of the file
		 * @param A reference to the file
		 */
		const TorrentFile & getFile(Uint32 idx) const;

		/**
		 * Calculate in which file(s) a Chunk lies. A list will
		 * get filled with the indices of all the files. The list gets cleared at
		 * the beginning. If something is wrong only the list will
		 * get cleared.
		 * @param chunk The index of the chunk
		 * @param file_list This list will be filled with all the indices
		 */
		void calcChunkPos(Uint32 chunk,QValueList<Uint32> & file_list) const;

		/**
		* Checks if torrent file is audio or video.
		**/
		bool isMultimedia() const;
		
		/// See if the torrent is private
		bool isPrivate() const {return priv_torrent;}
		
		///Gets a pointer to AnnounceList
		const TrackerTier* getTrackerList() const { return trackers; }
		
		/// Get the number of initial DHT nodes
		Uint32 getNumDHTNodes() const {return nodes.count();}
		
		/// Get a DHT node
		const kt::DHTNode & getDHTNode(Uint32 i) {return nodes[i];}
		
		/**
		 * Update the percentage of all files.
		 * @param bs The BitSet with all downloaded chunks
		 */
		void updateFilePercentage(const BitSet & bs);
		
		/**
		 * Update the percentage of a all files which have a particular chunk.
		 * @param bs The BitSet with all downloaded chunks
		 */
		void updateFilePercentage(Uint32 chunk,const BitSet & bs);
		

	private:
		void loadInfo(BDictNode* node);
		void loadTrackerURL(BValueNode* node);
		void loadPieceLength(BValueNode* node);
		void loadFileLength(BValueNode* node);
		void loadHash(BValueNode* node);
		void loadName(BValueNode* node);
		void loadFiles(BListNode* node);
		void loadNodes(BListNode* node);
		void loadAnnounceList(BNode* node);
		bool checkPathForDirectoryTraversal(const QString & p);
		
	private:
		TrackerTier* trackers;
		QString name_suggestion;
		Uint64 piece_length;
		Uint64 file_length;
		SHA1Hash info_hash;
		PeerID peer_id;
		QValueVector<SHA1Hash> hash_pieces;
		QValueVector<TorrentFile> files;
		QValueVector<kt::DHTNode> nodes;
		QString encoding;
		bool priv_torrent;
	};

}

#endif
