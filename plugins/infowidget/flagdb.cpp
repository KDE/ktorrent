/*
    SPDX-FileCopyrightText: 2007 Modestas Vainius <modestas@vainius.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "flagdb.h"
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QStandardPaths>

kt::FlagDBSource::FlagDBSource(const QString &pathPattern)
    : pathPattern(pathPattern)
{
}

kt::FlagDBSource::FlagDBSource()
{
}

QString kt::FlagDBSource::getPath(const QString &country) const
{
    // pathPattern = QStringLiteral("locale/l10n/%1/flag.png");
    // QStandardPaths::locate(QStandardPaths::GenericDataLocation, flagPath.arg(code));
    // example: /usr/share/locale/l10n/ru/flag.png (part of kde-runtime-data package)
    return pathPattern.arg(country);
}

const QPixmap &kt::FlagDB::nullPixmap = QPixmap();

kt::FlagDB::FlagDB(int preferredWidth, int preferredHeight)
    : preferredWidth(preferredWidth)
    , preferredHeight(preferredHeight)
    , sources()
    , db()
{
}

kt::FlagDB::FlagDB(const FlagDB &other)
    : preferredWidth(other.preferredWidth)
    , preferredHeight(other.preferredHeight)
    , sources(other.sources)
    , db(other.db)
{
}

kt::FlagDB::~FlagDB()
{
}

void kt::FlagDB::addFlagSource(const FlagDBSource &source)
{
    sources.append(source);
}

void kt::FlagDB::addFlagSource(const QString &pathPattern)
{
    addFlagSource(FlagDBSource(pathPattern));
}

const QList<kt::FlagDBSource> &kt::FlagDB::listSources() const
{
    return sources;
}

bool kt::FlagDB::isFlagAvailable(const QString &country)
{
    return getFlag(country).isNull();
}

const QPixmap &kt::FlagDB::getFlag(const QString &country)
{
    const QString &c = country.toLower();
    auto it = db.constFind(c);
    if (it != db.constEnd())
        return *it;

    QImage img;
    QPixmap pixmap;
    for (const FlagDBSource &s : qAsConst(sources)) {
        const QString &path = s.getPath(c);
        // e.g.: /usr/share/locale/l10n/ru/flag.png
        if (QFile::exists(path) && img.load(path)) {
            if (img.width() != preferredWidth || img.height() != preferredHeight) {
                const QImage &imgScaled = img.scaled(preferredWidth, preferredHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                if (!imgScaled.isNull()) {
                    pixmap = QPixmap::fromImage(imgScaled);
                    break;
                } else if (img.width() <= preferredWidth || img.height() <= preferredHeight) {
                    pixmap = QPixmap::fromImage(img);
                    break;
                }
            } else {
                pixmap = QPixmap::fromImage(img);
                break;
            }
        }
    }

    return (db[c] = (!pixmap.isNull()) ? pixmap : nullPixmap);
}
