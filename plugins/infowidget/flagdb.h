/*
    SPDX-FileCopyrightText: 2007 Modestas Vainius <modestas@vainius.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FLAGDB_H
#define FLAGDB_H

#include <QIcon>
#include <QMap>
#include <QString>

namespace kt
{
/**
@author Modestas Vainius
*/
class FlagDB
{
public:
    bool isFlagAvailable(const QString &country);
    const QIcon &getFlag(const QString &country);

private:
    static const QIcon &nullPixmap;
    QMap<QString, QIcon> db;
};
}

#endif
