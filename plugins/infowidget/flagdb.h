/*
    SPDX-FileCopyrightText: 2007 Modestas Vainius <modestas@vainius.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FLAGDB_H
#define FLAGDB_H

#include <QMap>
#include <QPixmap>
#include <QString>
#include <QStringList>

namespace kt
{
class FlagDBSource
{
public:
    FlagDBSource();
    FlagDBSource(const QString &pathPattern);
    QString getPath(const QString &country) const;

    const QString &getPathPattern()
    {
        return pathPattern;
    };

private:
    QString pathPattern;
};

/**
@author Modestas Vainius
*/
class FlagDB
{
public:
    FlagDB(int preferredWidth, int preferredHeight);
    FlagDB(const FlagDB &m);
    ~FlagDB();

    void addFlagSource(const FlagDBSource &source);
    void addFlagSource(const QString &pathPattern);
    const QList<FlagDBSource> &listSources() const;
    bool isFlagAvailable(const QString &country);
    const QPixmap &getFlag(const QString &country);

private:
    static const QPixmap &nullPixmap;
    int preferredWidth, preferredHeight;
    QList<FlagDBSource> sources;
    QMap<QString, QPixmap> db;
};
}

#endif
