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
#include <util/log.h>
#include <util/pipe.h>
#include <net/wakeuppipe.h>

using namespace net;
using namespace bt;


class WakeUpPipeTest : public QEventLoop
{
	Q_OBJECT
public:
	
public slots:

	
private slots:
	void initTestCase()
	{
		bt::InitLog("wakeuppipetest.log");
	}
	
	void cleanupTestCase()
	{
	}
	
	void testWakeUp()
	{
		Poll poll;
		WakeUpPipe p;
		p.wakeUp();
		
		poll.add(&p);
		QVERIFY(poll.poll() > 0);
	}
	
	void testEmptyWakeUp()
	{
		WakeUpPipe p;
		Poll poll;
		poll.add(&p);
		QVERIFY(poll.poll(100) == 0);
	}
	
private:
};

QTEST_MAIN(WakeUpPipeTest)

#include "wakeuppipetest.moc"