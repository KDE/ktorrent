/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ANTIP2P_H
#define ANTIP2P_H

#include <QVector>

#include <interfaces/blocklistinterface.h>
#include <util/constants.h>

namespace kt
{
struct IPBlock {
    bt::Uint32 ip1;
    bt::Uint32 ip2;

    IPBlock();
    IPBlock(const IPBlock &block);
    IPBlock(const QString &start, const QString &end);

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
    ~IPBlockList() override;

    bool blocked(const net::Address &addr) const override;

    /**
     * Overloaded function. Uses Uint32 IP to be checked
     **/
    bool isBlockedIP(bt::Uint32 ip);

    /**
     * Loads filter file
     * @param path The file to load
     * @return true upon success, false otherwise
     */
    bool load(const QString &path);

    /**
     * Add a single block
     * @param block
     */
    void addBlock(const IPBlock &block);

private:
    QVector<IPBlock> blocks;
};
}
#endif
