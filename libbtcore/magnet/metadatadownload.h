/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/


#ifndef BT_METADATADOWNLOAD_H
#define BT_METADATADOWNLOAD_H


#include <util/constants.h>
#include <util/bitset.h>
#include <QByteArray>

namespace bt
{
	class UTMetaData;
	
	const int METADATA_PIECE_SIZE = 16 * 1024;
	
	/**
		Handles the metadatadownload
	*/
	class MetadataDownload
	{
	public:
		MetadataDownload(UTMetaData* ext,Uint32 size);
		virtual ~MetadataDownload();
		
		/// A reject of a piece was received
		void reject(Uint32 piece);
		
		/**
			A piece was received
			@return true if all the data has been received
		*/
		bool data(Uint32 piece,const QByteArray & piece_data);
		
		/// Get the result
		const QByteArray & result() const {return metadata;}
		
	private:
		void download(Uint32 piece);
		void downloadNext();
		
	private:
		UTMetaData* ext;
		BitSet pieces;
		QByteArray metadata;
		Uint32 total_size;
	};

}

#endif // BT_METADATADOWNLOAD_H
