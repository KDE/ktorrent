/***************************************************************************
 *   Copyright (C) 2007 by Modestas Vainius <modestas@vainius.eu>          *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
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
        FlagDBSource(const QString& pathPattern);
        QString getPath(const QString& country) const;

        const QString& getPathPattern() { return pathPattern; };

    private:
        QString pathPattern;
    };

    /**
    @author Modestas Vainius
    */
    class FlagDB
    {
    public :
        FlagDB(int preferredWidth, int preferredHeight);
        FlagDB(const FlagDB& m);
        ~FlagDB();

        void addFlagSource(const FlagDBSource& source);
        void addFlagSource(const QString& pathPattern);
        const QList<FlagDBSource>& listSources() const;
        bool isFlagAvailable(const QString& country);
        const QPixmap& getFlag(const QString& country);
    private:
        static const QPixmap& nullPixmap;
        int preferredWidth, preferredHeight;
        QList<FlagDBSource> sources;
        QMap<QString, QPixmap> db;
    };
}

#endif
