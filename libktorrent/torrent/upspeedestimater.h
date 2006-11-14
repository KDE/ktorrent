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
#ifndef BTUPSPEEDESTIMATER_H
#define BTUPSPEEDESTIMATER_H

#include <qvaluelist.h>
#include <util/constants.h>

namespace bt
{

	/**
	 * @author Joris Guisson
	 *
	 * Measures upload speed.
	*/
	class UpSpeedEstimater
	{
		struct Entry
		{
			Uint32 bytes;
			TimeStamp start_time;
			Uint32 duration;
			bool data;
		};
	public:
		UpSpeedEstimater();
		virtual ~UpSpeedEstimater();

		/**
		 * Start sending bytes.
		 * @param bytes The number of bytes
		 * @param rec Wether to record or not (i.e. is this data)
		 */
		void writeBytes(Uint32 bytes,bool rec);
		
		/**
		 * The socket has finished sending bytes.
		 * @param bytes The number of bytes.
		 */
		void bytesWritten(Uint32 bytes);

		/**
		 * Update the upload speed estimater.
		 */
		void update();

		/// Get the upload rate 
		double uploadRate() const {return upload_rate;}
		
		/// Get the protocol overhead 
		double protocollOverhead() const {return proto_upload_rate;}
	private:
		double rate(QValueList<Entry> & el);
		
	private:
		double upload_rate;
		double proto_upload_rate;
		Uint32 accumulated_bytes;
		QValueList<Entry> outstanding_bytes;
		QValueList<Entry> written_bytes;
#ifdef MEASURE_PROTO_OVERHEAD
		QValueList<Entry> proto_bytes;
#endif
	};

}

#endif
