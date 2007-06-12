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
#ifndef BTBENCODER_H
#define BTBENCODER_H


#include <util/file.h>


namespace bt
{
	class File;

	/**
	 * @author Joris Guisson
	 *
	 * Interface for classes which wish to receive the output from a BEncoder.
	 */
	class BEncoderOutput
	{
	public:
		virtual ~BEncoderOutput() {}
		/**
		 * Write a string of characters.
		 * @param str The string
		 * @param len The length of the string
		 */
		virtual void write(const char* str,Uint32 len) = 0;
	};

	/**
	 * Writes the output of a bencoder to a file
	 */
	class BEncoderFileOutput : public BEncoderOutput
	{
		File* fptr;
	public:
		BEncoderFileOutput(File* fptr);

		void write(const char* str,Uint32 len);
	};
	
	/**
	 * Write the output of a BEncoder to a QByteArray
	 */
	class BEncoderBufferOutput : public BEncoderOutput
	{
		QByteArray & data;
		Uint32 ptr;
	public:
		BEncoderBufferOutput(QByteArray & data);

		void write(const char* str,Uint32 len);
	};


	/**
	 * @author Joris Guisson
	 * @brief Helper class to b-encode stuff.
	 * 
	 * This class b-encodes data. For more details about b-encoding, see
	 * the BitTorrent protocol docs. The data gets written to a BEncoderOutput
	 * thing.
	 */
	class BEncoder 
	{
		BEncoderOutput* out;
		bool del;
	public:
		/**
		 * Constructor, output gets written to a file.
		 * @param fptr The File to write to
		 */
		BEncoder(File* fptr);

		
		/**
		 * Constructor, output gets written to a BEncoderOutput object.
		 * @param out The BEncoderOutput
		 */
		BEncoder(BEncoderOutput* out);
		virtual ~BEncoder();

		/**
		 * Begin a dictionary.Should have a corresponding end call.
		 */
		void beginDict();
		
		/**
		 * Begin a list. Should have a corresponding end call.
		 */
		void beginList();
		
		/**
		 * Write an int
		 * @param val 
		 */
		void write(Uint32 val);

		/**
		 * Write an int64
		 * @param val
		 */
		void write(Uint64 val);
		
		/**
		 * Write a string
		 * @param str 
		 */
		void write(const QString & str);
		
		/**
		 * Write a QByteArray
		 * @param data 
		 */
		void write(const QByteArray & data);

		/**
		 * Write a data array
		 * @param data
		 * @param size of data
		 */
		void write(const Uint8* data,Uint32 size);
		
		/**
		 * End a beginDict or beginList call.
		 */
		void end();
	};

}

#endif
