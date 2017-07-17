/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#include "ipblocklist.h"
#include <QFile>
#include <util/log.h>
#include <util/constants.h>
#include <net/address.h>


using namespace bt;

namespace kt
{
    static Uint32 StringToUint32(const QString& ip)
    {
        bool test;
        Uint32 ret = ip.section(QLatin1Char('.'), 0, 0).toULongLong(&test);
        ret <<= 8;
        ret |= ip.section(QLatin1Char('.'), 1, 1).toULong(&test);
        ret <<= 8;
        ret |= ip.section(QLatin1Char('.'), 2, 2).toULong(&test);
        ret <<= 8;
        ret |= ip.section(QLatin1Char('.'), 3, 3).toULong(&test);

        return ret;
    }

    IPBlock::IPBlock() : ip1(0), ip2(0)
    {}

    IPBlock::IPBlock(const IPBlock& block) : ip1(block.ip1), ip2(block.ip2)
    {}

    IPBlock::IPBlock(const QString& start, const QString& end)
    {
        ip1 = StringToUint32(start);
        ip2 = StringToUint32(end);
    }

    IPBlockList::IPBlockList()
    {
    }

    IPBlockList::~IPBlockList()
    {
    }

    bool IPBlockList::blocked(const net::Address& addr) const
    {
        if (addr.protocol() == QAbstractSocket::IPv6Protocol || blocks.empty())
            return false;

        // Binary search the list of blocks which are sorted
        quint32 ip = addr.toIPv4Address();
        int begin = 0;
        int end = blocks.size() - 1;
        while (true)
        {
            if (begin == end)
                return blocks[begin].contains(ip);
            else if (begin == end - 1)
                return blocks[begin].contains(ip) || blocks[end].contains(ip);

            int pivot = begin + (end - begin) / 2;
            if (blocks[pivot].contains(ip))
                return true;
            else if (ip < blocks[pivot].ip1)
                end = pivot - 1; // continue in the range [begin, pivot - 1]
            else // ip > blocks[pivot].ip2
                begin = pivot + 1; // continue in the range [pivot + 1, end]
        }
        return false;
    }

    bool IPBlockList::load(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            Out(SYS_IPF | LOG_NOTICE) << "Cannot open " << path << ": " << file.errorString() << endl;
            return false;
        }

        // Note: the conversion process has sorted the blocks !
        int num_blocks = file.size() / sizeof(IPBlock);
        blocks.reserve(num_blocks);
        while (!file.atEnd() && blocks.size() < num_blocks)
        {
            IPBlock block;
            if (file.read((char*)&block, sizeof(IPBlock)) == sizeof(IPBlock))
                addBlock(block);
            else
                break;
        }

        Out(SYS_IPF | LOG_NOTICE) << "Loaded " << blocks.size() << " blocked IP ranges" << endl;
        return true;
    }

    void IPBlockList::addBlock(const IPBlock& block)
    {
        blocks.append(block);
    }

}
