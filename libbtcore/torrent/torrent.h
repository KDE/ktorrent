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
#include <QVector>
#include <QList>
#include <util/sha1hash.h>
#include <util/constants.h>
#include <interfaces/torrentinterface.h>
#include <peer/peerid.h>
#include <btcore_export.h>
#include "torrentfile.h"

class QTextCodec;


namespace bt
{
	class BNode;
	class BDictNode;
	class BListNode;


	struct TrackerTier
	{
		KUrl::List urls;
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
	 *
	 * Listener base class, to get notified when the priority of a file changes.
	 */
	class BTCORE_EXPORT FilePriorityListener 
	{
	public:
		virtual ~FilePriorityListener() {}
	
		virtual void downloadPriorityChanged(TorrentFile* tf,Priority newpriority,Priority oldpriority) = 0;
	};
	
	
	/**
	 * @author Joris Guisson
	 * @brief Loads a .torrent file
	 * 
	 * Loads a torrent file and calculates some miscelanious other data,
	 * like the info_hash and the peer_id.
	 */
	class BTCORE_EXPORT Torrent
	{
	public:
		Torrent();
		Torrent(const bt::SHA1Hash & hash);
		virtual ~Torrent();
		
		/**
		 * Set the FilePriorityListener
		 * @param l THe listener
		 */
		void setFilePriorityListener(FilePriorityListener* l) {file_prio_listener = l;}
		
		/**
		 * Called by TorrentFile when the priority changes
		 * @param tf The file
		 * @param newpriority The old priority 
		 * @param oldpriority The new priority
		 */
		void downloadPriorityChanged(TorrentFile* tf,Priority newpriority,Priority oldpriority);
		
		/**
		 * Called by TorrentFile when the percentage changes
		 * @param tf The file
		 * @param perc The percentage
		 */
		void filePercentageChanged(TorrentFile* tf,float perc);
		
		/**
		 * Called by TorrentFile when the preview state changes
		 * @param tf The file
		 * @param preview Whether preview is possible or not
		 */
		void filePreviewChanged(TorrentFile* tf,bool preview);

		/**
		 * Load a .torrent file.
		 * @param file The file
		 * @param verbose Whether to print information to the log
		 * @throw Error if something goes wrong
		 */
		void load(const QString & file,bool verbose);
		
		/**
		 * Load a .torrent file.
		 * @param data The data
		 * @param verbose Whether to print information to the log
		 * @throw Error if something goes wrong
		 */
		void load(const QByteArray & data,bool verbose);
		
		void debugPrintInfo();
		
		/// Return the comments in the torrent
		QString getComments() const {return comments;}
		
		/// Get the number of chunks.
		Uint32 getNumChunks() const {return hash_pieces.size();}
		
		/// Get the size of a chunk.
		Uint64 getChunkSize() const {return chunk_size;}
		
		/// Get the size of the last chunk
		Uint64 getLastChunkSize() const {return last_chunk_size;}
		
		/// Get the info_hash.
		const SHA1Hash & getInfoHash() const {return info_hash;}
		
		/// Get our peer_id.
		const PeerID & getPeerID() const {return peer_id;}
		
		/// Get the file size in number of bytes.
		Uint64 getTotalSize() const {return total_size;}
		
		/// Get the suggested name.
		QString getNameSuggestion() const {return name_suggestion;}
		
		/**
		 * Verify whether a hash matches the hash
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
		void calcChunkPos(Uint32 chunk,QList<Uint32> & file_list) const;

		/**
		* Checks if torrent file is audio or video.
		**/
		bool isMultimedia() const;
		
		/// See if the torrent is private
		bool isPrivate() const {return priv_torrent;}
		
		/// Is the torrent loaded
		bool isLoaded() const {return loaded;}
		
		///Gets a pointer to AnnounceList
		const TrackerTier* getTrackerList() const { return trackers; }
		
		/// Get the number of initial DHT nodes
		Uint32 getNumDHTNodes() const {return nodes.count();}
		
		/// Get a DHT node
		const DHTNode & getDHTNode(Uint32 i) {return nodes[i];}
		
		/**
		 * Update the percentage of all files.
		 * @param cman The ChunkManager
		 */
		void updateFilePercentage(ChunkManager & cman);
		
		/**
		 * Update the percentage of a all files which have a particular chunk.
		 * @param cman The ChunkManager
		 */
		void updateFilePercentage(Uint32 chunk,ChunkManager & cman);
		
		/**
		 * Get the list with web seed URL's
		 */
		const KUrl::List & getWebSeeds() const {return web_seeds;}
		
		/// Change the text codec
		void changeTextCodec(QTextCodec* codec);
		
		/// Get the text codec
		const QTextCodec* getTextCodec() {return text_codec;}
		
		/// Set the monitor
		void setMonitor(MonitorInterface* m) {tmon = m;}
		
		/// Get the metadata
		const QByteArray & getMetaData() const {return metadata;}

	private:
		void loadInfo(BDictNode* node);
		void loadTrackerURL(const QString & s);
		void loadHash(BDictNode* dict);
		void loadFiles(BListNode* node);
		void loadNodes(BListNode* node);
		void loadAnnounceList(BNode* node);
		void loadWebSeeds(BListNode* node);
		bool checkPathForDirectoryTraversal(const QString & p);
		
	private:
		TrackerTier* trackers;
		QString name_suggestion;
		QByteArray unencoded_name;
		Uint64 chunk_size;
		Uint64 last_chunk_size;
		Uint64 total_size;
		SHA1Hash info_hash;
		PeerID peer_id;
		QVector<SHA1Hash> hash_pieces;
		QVector<TorrentFile> files;
		QVector<DHTNode> nodes;
		QTextCodec* text_codec;
		bool priv_torrent;
		KUrl::List web_seeds;
		FilePriorityListener* file_prio_listener;
		mutable Uint32 pos_cache_chunk;
		mutable Uint32 pos_cache_file;
		MonitorInterface* tmon;
		QString comments;
		QByteArray metadata;
		bool loaded;
	};

}

#endif
