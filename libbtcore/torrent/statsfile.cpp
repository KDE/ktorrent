/***************************************************************************
 *   Copyright (C) 2005-2010 by Joris Guisson                              *
 *   joris.guisson@gmail.com                                               *
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
#include "statsfile.h"

#include <util/log.h>
#include <util/functions.h>
#include <KConfigGroup>


namespace bt
{

	StatsFile::StatsFile(const QString &filename)
	{
		cfg = KSharedConfig::openConfig(filename);
	}

	StatsFile::~StatsFile()
	{
	}
	
	void StatsFile::write(const QString &key, const QString &value)
	{
		cfg->group(QString()).writeEntry(key,value);
	}
	
	QString StatsFile::readString(const QString &key)
	{
		KConfigGroup g = cfg->group(QString());
		return g.readEntry(key).trimmed();
	}
	
	Uint64 StatsFile::readUint64(const QString &key)
	{
		bool ok = true;
		Uint64 val = readString(key).toULongLong(&ok);
		return val;
	}
	
	int StatsFile::readInt(const QString &key)
	{
		bool ok = true;
		int val = readString(key).toInt(&ok);
		return val;
	}
	
	bool StatsFile::readBoolean(const QString &key)
	{
		return (bool) readInt(key);
	}
	
	unsigned long StatsFile::readULong(const QString &key)
	{
		bool ok = true;
		return readString(key).toULong(&ok);
	}
	
	float StatsFile::readFloat(const QString &key)
	{
		bool ok = true;
		return readString(key).toFloat(&ok);
	}
	
	void StatsFile::sync()
	{
		cfg->sync();
	}
	
	bool StatsFile::hasKey(const QString& key) const
	{
		return cfg->group(QString()).hasKey(key);
	}
	
}
