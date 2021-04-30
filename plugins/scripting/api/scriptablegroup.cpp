/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scriptablegroup.h"
#include <dbus/dbus.h>
#include <interfaces/torrentinterface.h>
#include <util/log.h>
#include <util/sha1hash.h>

using namespace bt;

namespace kt
{
ScriptableGroup::ScriptableGroup(const QString &name, const QString &icon, const QString &path, Kross::Object::Ptr script, DBus *api)
    : kt::Group(name, MIXED_GROUP, path)
    , script(script)
    , api(api)
{
    setIconByName(icon);
}

ScriptableGroup::~ScriptableGroup()
{
}

bool ScriptableGroup::isMember(bt::TorrentInterface *tor)
{
    QVariantList args;
    args << tor->getInfoHash().toString();
    QVariant ret = script->callMethod(QStringLiteral("isMember"), args);
    return ret.toBool();
}

}
