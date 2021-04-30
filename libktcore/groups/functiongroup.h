/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFUNCTIONGROUP_H
#define KTFUNCTIONGROUP_H

#include "group.h"

namespace kt
{
typedef bool (*IsMemberFunction)(TorrentInterface *tor);

/**
    Group which calls a function pointer to test for membership
*/
template<IsMemberFunction fn> class FunctionGroup : public Group
{
public:
    FunctionGroup(const QString &name, const QString &icon, int flags, const QString &path)
        : Group(name, flags, path)
    {
        setIconByName(icon);
    }

    ~FunctionGroup() override
    {
    }

    bool isMember(TorrentInterface *tor) override
    {
        if (!tor)
            return false;
        else
            return fn(tor);
    }
};

}

#endif
