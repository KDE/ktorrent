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

#ifndef BTMMAPFILE_H
#define BTMMAPFILE_H

#include <QFile>
#include <QString>

#include <util/constants.h>
#include <ktcore_export.h>

namespace bt
{

    /**
     * @author Joris Guisson
     * @brief Memory mapped file
     *
     * This class allows to access memory mapped files. It's pretty similar to
     * File.
    */
    class KTCORE_EXPORT MMapFile
    {
    public:
        MMapFile();
        ~MMapFile();

        /**
         * Open the file. If mode is write and the file doesn't exist, it will
         * be created.
         * @param file Filename
         * @param mode Mode (READ, WRITE or RW)
         * @return true upon succes
         */
        bool open(const QString& file, QIODevice::OpenModeFlag mode);

        /**
         * Close the file. Undoes the memory mapping.
         */
        void close();

        /**
         * Flush the file.
         */
        void flush();

        /**
         * Write a bunch of data.
         * @param buf The data
         * @param size Size of the data
         * @return The number of bytes written
         */
        Uint32 write(const void* buf, Uint32 size);

        /**
         * Read a bunch of data
         * @param buf The buffer to store the data
         * @param size Size of the buffer
         * @return The number of bytes read
         */
        Uint32 read(void* buf, Uint32 size);

        enum SeekPos
        {
            BEGIN,
            END,
            CURRENT
        };

        /**
         * Seek in the file.
         * @param from Position to seek from
         * @param num Number of bytes to move
         * @return New position
         */
        Uint64 seek(SeekPos from, Int64 num);

        /// Check to see if we are at the end of the file.
        bool eof() const;

        /// Get the current position in the file.
        Uint64 tell() const;

        /// Get the error string.
        QString errorString() const;

        /// Get the file size
        Uint64 getSize() const;


        /**
         * Get a pointer to the mmapped region of data.
         * @param off Offset into buffer, if invalid 0 will be returned
         * @return Pointer to a location in the mmapped region
         */
        Uint8* getData(Uint64 off);

        /// Gets the data pointer
        void* getDataPointer() { return data; }
    private:
        void growFile(Uint64 new_size);

    private:
        QFile* fptr;
        Uint8* data;
        Uint64 size;    // size of mmapping
        Uint64 file_size; // size of file
        Uint64 ptr;
        QString filename;
        QIODevice::OpenModeFlag mode;
    };

}


#endif
