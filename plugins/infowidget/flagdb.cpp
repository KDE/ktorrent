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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QStandardPaths>
#include "flagdb.h"

kt::FlagDBSource::FlagDBSource(const QString& pathPattern)
    : pathPattern(pathPattern)
{
}

kt::FlagDBSource::FlagDBSource()
{
}

QString kt::FlagDBSource::getPath(const QString& country) const
{
    //pathPattern = QStringLiteral("locale/l10n/%1/flag.png");
    //QStandardPaths::locate(QStandardPaths::GenericDataLocation, flagPath.arg(code));
    // example: /usr/share/locale/l10n/ru/flag.png (part of kde-runtime-data package)
    return pathPattern.arg(country);
}

const QPixmap& kt::FlagDB::nullPixmap = QPixmap();

kt::FlagDB::FlagDB(int preferredWidth, int preferredHeight)
    : preferredWidth(preferredWidth),
      preferredHeight(preferredHeight),
      sources(),
      db()
{
}


kt::FlagDB::FlagDB(const FlagDB& other)
    : preferredWidth(other.preferredWidth),
      preferredHeight(other.preferredHeight),
      sources(other.sources),
      db(other.db)
{
}

kt::FlagDB::~FlagDB()
{
}

void kt::FlagDB::addFlagSource(const FlagDBSource& source)
{
    sources.append(source);
}

void kt::FlagDB::addFlagSource(const QString& pathPattern)
{
    addFlagSource(FlagDBSource(pathPattern));
}

const QList<kt::FlagDBSource>& kt::FlagDB::listSources() const
{
    return sources;
}

bool kt::FlagDB::isFlagAvailable(const QString& country)
{
    return getFlag(country).isNull();
}

const QPixmap& kt::FlagDB::getFlag(const QString& country)
{
    const QString& c = country.toLower();
    auto it = db.constFind(c);
    if (it != db.constEnd())
        return *it;

    QImage img;
    QPixmap pixmap;
    foreach (const FlagDBSource& s, sources)
    {
        const QString& path = s.getPath(c);
        //e.g.: /usr/share/locale/l10n/ru/flag.png
        if (QFile::exists(path) && img.load(path))
        {
            if (img.width() != preferredWidth || img.height() != preferredHeight)
            {
                const QImage& imgScaled = img.scaled(preferredWidth, preferredHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                if (!imgScaled.isNull())
                {
                    pixmap = QPixmap::fromImage(imgScaled);
                    break;
                }
                else if (img.width() <= preferredWidth || img.height() <= preferredHeight)
                {
                    pixmap = QPixmap::fromImage(img);
                    break;
                }
            }
            else
            {
                pixmap = QPixmap::fromImage(img);
                break;
            }
        }
    }

    return (db[c] = (!pixmap.isNull()) ? pixmap : nullPixmap);
}
