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

#include <util/log.h>
#include <util/sha1hash.h>
#include <interfaces/torrentinterface.h>
#include <dbus/dbus.h>
#include "scriptablegroup.h"

using namespace bt;

namespace kt
{

    ScriptableGroup::ScriptableGroup(const QString& name, const QString& icon, const QString& path, Kross::Object::Ptr script, DBus* api) : kt::Group(name, MIXED_GROUP, path), script(script), api(api)
    {
        setIconByName(icon);
    }

    ScriptableGroup::~ScriptableGroup()
    {}

    bool ScriptableGroup::isMember(bt::TorrentInterface* tor)
    {
        QVariantList args;
        args << tor->getInfoHash().toString();
        QVariant ret = script->callMethod(QStringLiteral("isMember"), args);
        return ret.toBool();
    }


}
