/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "statsfile.h"

#include "globals.h"
#include <util/log.h>
#include <util/functions.h>

#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>

namespace bt
{

	StatsFile::StatsFile(const QString &filename)
	:m_filename(filename),m_file(filename)
	{
		readSync();
	}

	StatsFile::~StatsFile()
	{
		close();
	}

	void StatsFile::close()
	{
		m_file.close();
	}
	
	void StatsFile::write(const QString &key, const QString &value)
	{
		m_values.insert(key.trimmed(), value.trimmed());
	}
	
	QString StatsFile::readString(const QString &key)
	{
		return m_values[key].trimmed();
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
	
	float bt::StatsFile::readFloat(const QString &key)
	{
		bool ok = true;
		return readString(key).toFloat(&ok);
	}
	
	void StatsFile::readSync()
	{
		if (!m_file.open(QIODevice::ReadOnly))
			return;
		
		QTextStream in(&m_file);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			m_values.insert(line.section('=',0,0).trimmed(), line.section('=',1,1).trimmed());
		}
		close();
	}
	
	void StatsFile::writeSync()
	{
		if (!m_file.open(QIODevice::WriteOnly))
			return;
		QTextStream out(&m_file);
		QMap<QString, QString>::iterator it = m_values.begin();
		while(it!=m_values.end())
		{
			out << it.key() << "=" << it.value() << ::endl;
			++it;
		}
		close();
	}
	
}
