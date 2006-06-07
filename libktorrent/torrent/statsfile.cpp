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

	StatsFile::StatsFile(QString filename)
	:m_filename(filename)
	{
		m_file.setName(filename);
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
	
	void StatsFile::write(QString key, QString value)
	{
		m_values.insert(key.stripWhiteSpace(), value.stripWhiteSpace());
	}
	
	QString StatsFile::readString(QString key)
	{
		return m_values[key].stripWhiteSpace();
	}
	
	Uint64 StatsFile::readUint64(QString key)
	{
		bool ok = true;
		Uint64 val = readString(key).toULongLong(&ok);
		return val;
	}
	
	int StatsFile::readInt(QString key)
	{
		bool ok = true;
		int val = readString(key).toInt(&ok);
		return val;
	}
	
	bool StatsFile::readBoolean(QString key)
	{
		return (bool) readInt(key);
	}
	
	unsigned long StatsFile::readULong(QString key)
	{
		bool ok = true;
		return readString(key).toULong(&ok);
	}
	
	float bt::StatsFile::readFloat( QString key )
	{
		bool ok = true;
		return readString(key).toFloat(&ok);
	}
	
	void StatsFile::readSync()
	{
		if (!m_file.open(IO_ReadOnly))
			return;
		
		QTextStream in(&m_file);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			QString tmp = line.left(line.find('='));
			m_values.insert(tmp, line.mid(tmp.length()+1));
		}
		close();
	}
	
	void StatsFile::writeSync()
	{
		if (!m_file.open(IO_WriteOnly))
			return;
		QTextStream out(&m_file);
		QMap<QString, QString>::iterator it = m_values.begin();
		while(it!=m_values.end())
		{
			out << it.key() << "=" << it.data() << ::endl;
			++it;
		}
		close();
	}
	
}
