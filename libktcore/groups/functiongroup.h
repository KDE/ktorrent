/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef KTFUNCTIONGROUP_H
#define KTFUNCTIONGROUP_H

#include "group.h"

namespace kt
{

    typedef bool (*IsMemberFunction)(TorrentInterface* tor);

    /**
        Group which calls a function pointer to test for membership
    */
    template <IsMemberFunction fn>
    class FunctionGroup : public Group
    {
    public:
        FunctionGroup(const QString& name, const QString& icon, int flags, const QString& path)
            : Group(name, flags, path)
        {
            setIconByName(icon);
        }

        ~FunctionGroup() {}

        bool isMember(TorrentInterface* tor) override
        {
            if (!tor)
                return false;
            else
                return fn(tor);
        }

    };

}

#endif
