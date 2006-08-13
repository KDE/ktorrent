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
#ifndef KTTORRENTFILEINTERFACE_H
#define KTTORRENTFILEINTERFACE_H

#include <qobject.h>
#include <qstring.h>
#include <util/constants.h>

namespace kt
{
	using bt::Uint32;
	using bt::Uint64;
	using bt::Priority;
	using bt::PREVIEW_PRIORITY;
	using bt::FIRST_PRIORITY;
	using bt::NORMAL_PRIORITY;
	using bt::LAST_PRIORITY;
	using bt::EXCLUDED;

	/**
	 * @author Joris Guisson
	 * @brief Interface for a file in a multifile torrent
	 *
	 * This class is the interface for a file in a multifile torrent.
	*/
	class TorrentFileInterface : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Constructor, set the path and size.
		 * @param path The path 
		 * @param size The size
		 */
		TorrentFileInterface(const QString & path,Uint64 size);
		virtual ~TorrentFileInterface();

		/// Get the path of the file
		QString getPath() const {return path;}

		/// Get the size of the file
		Uint64 getSize() const {return size;}

		/// Get the index of the first chunk in which this file lies
		Uint32 getFirstChunk() const {return first_chunk;}
				
		/// Get the last chunk of the file
		Uint32 getLastChunk() const {return last_chunk;}

		/// See if the TorrentFile is null.
		bool isNull() const {return path.isNull();}

		/// Set wether we have to not download this file
		virtual void setDoNotDownload(bool dnd) = 0;

		/// Wether or not we have to not download this file
		virtual bool doNotDownload() const = 0;

		/// Checks if this file is multimedial
		virtual bool isMultimedia() const = 0;

		/// Gets the current priority of the torrent
		virtual Priority getPriority() const {return priority;}

		/// Sets the priority of the torrent
		virtual void setPriority(Priority newpriority = NORMAL_PRIORITY) = 0;
		
		/// Wheather to emit signal when dl status changes or not.
		virtual void setEmitDownloadStatusChanged(bool show) = 0;
		
		/// Emits signal dlStatusChanged. Use it only with FileSelectDialog!
		virtual void emitDownloadStatusChanged() = 0;
		
		/// Did this file exist before the torrent was loaded by KT
		bool isPreExistingFile() const {return preexisting;}
		
		/// Set wether this file is preexisting
		void setPreExisting(bool pe) {preexisting = pe;}
		
		/// Get the % of the file which is downloaded
		float getDownloadPercentage() const;
		
		/// See if preview is available
		bool isPreviewAvailable() const {return preview;}
		
	signals:
		/**
		 * Emitted when the download percentage has been changed.
		 * @param p The new percentage
		 */
		void downloadPercentageChanged(float p);
		
		/**
		 * Emitted when the preview becomes available or not.
		 * @param available 
		 */
		void previewAvailable(bool available);
		
	protected:
		QString path;
		Uint64 size;
		Uint32 first_chunk;
		Uint32 last_chunk;
		Uint32 num_chunks_downloaded;
		Priority priority;
		bool preexisting;
		bool m_emitDlStatusChanged;
		bool preview;
	};

}

#endif
