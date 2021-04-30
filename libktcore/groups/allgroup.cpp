/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "allgroup.h"
#include <KLocalizedString>

namespace kt
{
AllGroup::AllGroup()
    : Group(i18n("All Torrents"), MIXED_GROUP, QStringLiteral("/all"))
{
    setIconByName(QStringLiteral("folder"));
}

AllGroup::~AllGroup()
{
}

bool AllGroup::isMember(TorrentInterface *tor)
{
    return tor != 0;
}

}
