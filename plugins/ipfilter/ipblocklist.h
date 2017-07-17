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

#ifndef ANTIP2P_H
#define ANTIP2P_H

#include <QVector>

#include <util/constants.h>
#include <interfaces/blocklistinterface.h>

namespace kt
{

    struct IPBlock
    {
        bt::Uint32 ip1;
        bt::Uint32 ip2;

        IPBlock();
        IPBlock(const IPBlock& block);
        IPBlock(const QString& start, const QString& end);

        bool contains(bt::Uint32 ip) const
        {
            return ip1 <= ip && ip <= ip2;
        }
    };

    /**
     * @author Ivan Vasic <ivasic@gmail.com>
     * @brief This class is used to manage anti-p2p filter list, so called level1.
     */
    class IPBlockList : public bt::BlockListInterface
    {
    public:
        IPBlockList();
        ~IPBlockList();

        bool blocked(const net::Address& addr) const override;

        /**
         * Overloaded function. Uses Uint32 IP to be checked
         **/
        bool isBlockedIP(bt::Uint32 ip);

        /**
         * Loads filter file
         * @param path The file to load
         * @return true upon success, false otherwise
         */
        bool load(const QString& path);

        /**
         * Add a single block
         * @param block
         */
        void addBlock(const IPBlock& block);

    private:
        QVector<IPBlock> blocks;
    };
}
#endif
