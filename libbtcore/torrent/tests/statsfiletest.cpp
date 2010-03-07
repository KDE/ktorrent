/***************************************************************************
*   Copyright (C) 2010 by Joris Guisson                                   *
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

#include <QtTest>
#include <QObject>
#include <time.h>
#include <util/log.h>
#include <torrent/statsfile.h>
#include <KSharedConfig>
#include <KConfigGroup>

using namespace bt;

QString test_data = "ASSURED_DOWNLOAD_SPEED=0\n\
ASSURED_UPLOAD_SPEED=0\n\
AUTOSTART=0\n\
CUSTOM_OUTPUT_NAME=0\n\
DHT=1\n\
DISPLAY_NAME=\n\
DOWNLOAD_LIMIT=0\n\
ENCODING=UTF-8\n\
IMPORTED=0\n\
MAX_RATIO=3.00\n\
MAX_SEED_TIME=30\n\
OUTPUTDIR=/home/joris/ktorrent/downloads/\n\
PRIORITY=4\n\
QM_CAN_START=0\n\
RESTART_DISK_PREALLOCATION=0\n\
RUNNING_TIME_DL=7042\n\
RUNNING_TIME_UL=7042\n\
TIME_ADDED=1265650102\n\
UPLOADED=0\n\
UPLOAD_LIMIT=0\n\
URL=file:///home/joris/tmp/Killers.torrent\n\
UT_PEX=1\n";


class StatsFileTest : public QEventLoop
{
	Q_OBJECT
public:

	
private slots:
	void initTestCase()
	{
		bt::InitLog("statsfiletest.log",false,false);
		QVERIFY(file.open());
		file.setAutoRemove(true);
		QTextStream out(&file);
		out << test_data;
		
		QStringList lines = test_data.split("\n");
		foreach (const QString & line,lines)
		{
			QStringList sl = line.split("=");
			if (sl.count() == 2)
			{
				keys.append(sl[0]);
				values.append(sl[1]);
			}
		}
	}
	
	void cleanupTestCase()
	{
	}
	
	void testRead()
	{
		StatsFile st(file.fileName());
		
		int idx = 0;
		foreach (const QString & key,keys)
		{
			QVERIFY(st.hasKey(key));
			QVERIFY(st.readString(key) == values[idx++]);
		}
		
		QVERIFY(st.readInt("RUNNING_TIME_DL") == 7042);
		QVERIFY(st.readInt("RUNNING_TIME_UL") == 7042);
		QVERIFY(st.readBoolean("DHT") == true);
	}
	
	void testWrite()
	{
		StatsFile sta(file.fileName());
		sta.write("DINGES","1234");
		sta.sync();
		
		StatsFile stb(file.fileName());
		QVERIFY(stb.readInt("DINGES") == 1234);
	}
	
	
private:
	QTemporaryFile file;
	QStringList keys;
	QStringList values;
};

QTEST_MAIN(StatsFileTest)

#include "statsfiletest.moc"