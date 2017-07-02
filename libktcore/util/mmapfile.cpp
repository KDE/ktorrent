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

#include <config-ktcore.h>

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

#include <QFile>

#include <KFileItem>
#include <KLocalizedString>

#include <util/error.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "mmapfile.h"

namespace bt
{

    MMapFile::MMapFile() : fptr(nullptr), data(0), size(0), file_size(0), ptr(0), mode(QIODevice::ReadOnly)
    {}


    MMapFile::~MMapFile()
    {
        if (fptr)
            close();
    }

    bool MMapFile::open(const QString& file, QIODevice::OpenModeFlag mode)
    {
        // close already open file
        if (fptr && fptr->isOpen())
        {
            close();
        }

        // setup flags
        int mmap_flag = 0;
        switch (mode)
        {
        case QIODevice::ReadOnly:
            mmap_flag = PROT_READ;
            break;
        case QIODevice::WriteOnly:
            mmap_flag = PROT_WRITE;
            break;
        default:
        case QIODevice::ReadWrite:
            mmap_flag = PROT_READ | PROT_WRITE;
            break;
        }

        fptr = new QFile(file);
        // open the file
        if (!(fptr->open(mode)))
        {
            delete fptr;
            fptr = nullptr;
            return false;
        }

        // read the file size
        this->size = fptr->size();
        this->mode = mode;

        file_size = fptr->size();
        filename = file;

        // mmap the file
#ifndef Q_WS_WIN
        int fd = fptr->handle();
#ifdef HAVE_MMAP64
        data = (Uint8*)mmap64(0, size, mmap_flag, MAP_SHARED, fd, 0);
#else
        data = (Uint8*)mmap(0, size, mmap_flag, MAP_SHARED, fd, 0);
#endif
        if (data == MAP_FAILED)
        {
            ::close(fd);
            data = 0;
            fd = -1;
            ptr = 0;
            return false;
        }
        ptr = 0;
        return true;
#else // Q_WS_WIN
        data = (Uint8*)fptr->map(0, size);

        if (!data)
        {
            fptr->close();
            delete fptr;
            fptr = nullptr;
            return false;
        }
        ptr = 0;
        return true;
#endif
    }

    void MMapFile::close()
    {
        if (fptr)
        {
#ifndef Q_WS_WIN
#ifdef HAVE_MUNMAP64
            munmap64(data, size);
#else
            munmap(data, size);
#endif
#else
            fptr->unmap(data);
#endif
            fptr->close();
            delete fptr;
            fptr = 0;
            ptr = size = 0;
            data = 0;
            filename = QString::null;
        }
    }

    void MMapFile::flush()
    {
        if (fptr)
#ifndef Q_WS_WIN
            msync(data, size, MS_SYNC);
#else
            FlushViewOfFile(data, size);
#endif
    }

    Uint32 MMapFile::write(const void* buf, Uint32 buf_size)
    {
        if (!fptr || mode == QIODevice::ReadOnly)
            return 0;

        // check if data fits in memory mapping
        if (ptr + buf_size > size)
            throw Error(i18n("Cannot write beyond end of the mmap buffer."));

        Out(SYS_GEN | LOG_DEBUG) << "MMapFile::write : " << (ptr + buf_size) << " " << file_size << endl;
        // enlarge the file if necessary
        if (ptr + buf_size > file_size)
        {
            growFile(ptr + buf_size);
        }

        // memcpy data
        memcpy(&data[ptr], buf, buf_size);
        // update ptr
        ptr += buf_size;
        // update file size if necessary
        if (ptr >= size)
            size = ptr;

        return buf_size;
    }

    void MMapFile::growFile(Uint64 new_size)
    {
        Out(SYS_GEN | LOG_DEBUG) << "Growing file to " << new_size << " bytes " << endl;
        Uint64 to_write = new_size - file_size;
        // jump to the end of the file
        fptr->seek(fptr->size());

        Uint8 buf[1024];
        memset(buf, 0, 1024);
        // write data until to_write is 0
        while (to_write > 0)
        {
            ssize_t w = fptr->write((const char*)buf, to_write > 1024 ? 1024 : to_write);
            if (w > 0)
                to_write -= w;
            else if (w < 0)
                break;
        }
        file_size = new_size;
    }

    Uint32 MMapFile::read(void* buf, Uint32 buf_size)
    {
        if (!fptr || mode == QIODevice::WriteOnly)
            return 0;

        // check if we aren't going to read past the end of the file
        Uint32 to_read = ptr + buf_size >= size ? size - ptr : buf_size;
        // read data
        memcpy(buf, data + ptr, to_read);
        ptr += to_read;
        return to_read;
    }

    Uint64 MMapFile::seek(SeekPos from, Int64 num)
    {
        switch (from)
        {
        case BEGIN:
            if (num > 0)
                ptr = num;
            if (ptr >= size)
                ptr = size - 1;
            break;
        case END:
        {
            Int64 np = (size - 1) + num;
            if (np < 0)
            {
                ptr = 0;
                break;
            }
            if (np >= (Int64) size)
            {
                ptr = size - 1;
                break;
            }
            ptr = np;
        }
        break;
        case CURRENT:
        {
            Int64 np = ptr + num;
            if (np < 0)
            {
                ptr = 0;
                break;
            }
            if (np >= (Int64) size)
            {
                ptr = size - 1;
                break;
            }
            ptr = np;
        }
        break;
        }
        return ptr;
    }

    bool MMapFile::eof() const
    {
        return ptr >= size;
    }

    Uint64 MMapFile::tell() const
    {
        return ptr;
    }

    QString MMapFile::errorString() const
    {
        return QString::fromUtf8(strerror(errno));
    }

    Uint64 MMapFile::getSize() const
    {
        return size;
    }

    Uint8* MMapFile::getData(Uint64 off)
    {
        if (off >= size)
            return nullptr;
        return &data[off];
    }
}

