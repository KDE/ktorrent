/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTALLGROUP_H
#define KTALLGROUP_H

#include <groups/group.h>

namespace kt
{
/**
    @author Joris Guisson <joris.guisson@gmail.com>
*/
class AllGroup : public Group
{
public:
    AllGroup();
    ~AllGroup() override;

    bool isMember(TorrentInterface *tor) override;
};

}

#endif
