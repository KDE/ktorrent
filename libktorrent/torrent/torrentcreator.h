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
#ifndef BTTORRENTCREATOR_H
#define BTTORRENTCREATOR_H

#include <qstringlist.h>
#include "torrent.h"
#include <util/sha1hash.h>

namespace bt
{
	class BEncoder;
	class TorrentControl;

	/**
	 * @author Joris Guisson
	 * @brief Class to generate torrent files
	 *
	 * This class generates torrent files.
	 * It also allows to create a TorrentControl object, so
	 * that we immediately can start to share the torrent.
	 */
	class TorrentCreator
	{
		// input values
		QString target;
		QStringList trackers;
		int chunk_size;
		QString name,comments;
		// calculated values
		Uint32 num_chunks;
		Uint64 last_size;
		QValueList<TorrentFile> files;
		QValueList<SHA1Hash> hashes;
		//
		Uint32 cur_chunk;
		bool priv;
		Uint64 tot_size;
		bool decentralized;
	public:
		/**
		 * Constructor.
		 * @param target The file or directory to make a torrent of
		 * @param trackers A list of tracker urls
		 * @param chunk_size The size of each chunk
		 * @param name The name suggestion
		 * @param comments The comments field of the torrent
		 * @param priv Private torrent or not
		 */
		TorrentCreator(const QString & target,const QStringList & trackers,
					   Uint32 chunk_size,const QString & name,
					   const QString & comments,bool priv,bool decentralized);
		virtual ~TorrentCreator();

		
		/**
		 * Calculate the hash of a chunk, this function should be called
		 * until it returns true. We do it this way so that the calling
		 * function can display a progress dialog. 
		 * @return true if all hashes are calculated, false otherwise
		 */
		bool calculateHash();

		/// Get the number of chunks
		Uint32 getNumChunks() const {return num_chunks;}
		
		/**
		 * Save the torrent file.
		 * @param url Filename
		 * @throw Error if something goes wrong
		 */
		void saveTorrent(const QString & url);
		
		/**
		 * Make a TorrentControl object for this torrent.
		 * This will also create the files :
		 * data_dir/index
		 * data_dir/torrent
		 * data_dir/cache (symlink to target)
		 * @param data_dir The data directory
		 * @throw Error if something goes wrong
		 * @return The newly created object
		 */
		TorrentControl* makeTC(const QString & data_dir);

	private:
		void saveInfo(BEncoder & enc);
		void saveFile(BEncoder & enc,const TorrentFile & file);
		void savePieces(BEncoder & enc);
		void buildFileList(const QString & dir);
		bool calcHashSingle();
		bool calcHashMulti();
	};

}

#endif
