/*
    SPDX-FileCopyrightText: 2007 Modestas Vainius <modestas@vainius.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "flagdb.h"

#include <KCountryFlagEmojiIconEngine>

const QIcon &kt::FlagDB::nullPixmap = QPixmap();

bool kt::FlagDB::isFlagAvailable(const QString &country)
{
    return getFlag(country).isNull();
}

const QIcon &kt::FlagDB::getFlag(const QString &country)
{
    const QString &c = country.toLower();
    auto it = db.constFind(c);
    if (it != db.constEnd()) {
        return *it;
    }

    QIcon icon(new KCountryFlagEmojiIconEngine(country));

    return (db[c] = (!icon.isNull()) ? icon : nullPixmap);
}
