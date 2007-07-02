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

#include <qfile.h>
#include <qimage.h>
#include <kstandarddirs.h>
#include "flagdb.h"

kt::FlagDBSource::FlagDBSource(const char* type, const QString& pathPattern)
	: type(type), pathPattern(pathPattern) 
{
}

kt::FlagDBSource::FlagDBSource(const QString& pathPattern)
	: type(NULL), pathPattern(pathPattern) 
{
}

kt::FlagDBSource::FlagDBSource()
	: type(NULL), pathPattern()
{
}

QString kt::FlagDBSource::FlagDBSource::getPath(const QString& country) const
{
	if (type) {
		return locate(type, pathPattern.arg(country));
	} else {
		return pathPattern.arg(country);
	}
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

void kt::FlagDB::addFlagSource(const char* type, const QString& pathPattern)
{
	addFlagSource(FlagDBSource(type, pathPattern));
}

const QValueList<kt::FlagDBSource>& kt::FlagDB::listSources() const
{
	return sources;
}

bool kt::FlagDB::isFlagAvailable(const QString& country)
{
	return getFlag(country).isNull();
}

const QPixmap& kt::FlagDB::getFlag(const QString& country)
{
	const QString& c = country.lower();
	if (!db.contains(c)) {
		QImage img;
		QPixmap pixmap;
		for (QValueList<FlagDBSource>::const_iterator it = sources.constBegin(); it != sources.constEnd(); it++) {
			const QString& path = (*it).getPath(c);
			if (QFile::exists(path)) {
				if (img.load(path)) {
					if (img.width() != preferredWidth || img.height() != preferredHeight) {
						const QImage& imgScaled = img.smoothScale(preferredWidth, preferredHeight, QImage::ScaleMin);
						if (!imgScaled.isNull()) {
							pixmap.convertFromImage(imgScaled);
							break;
						} else if (img.width() <= preferredWidth || img.height() <= preferredHeight) {
							pixmap.convertFromImage(img);
							break;
						}
					}
				}
			}
		}

		db[c] = (!pixmap.isNull()) ? pixmap : nullPixmap;
	}
	return db[c];
}
