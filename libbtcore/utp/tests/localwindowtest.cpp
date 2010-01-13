#include <QtTest>
#include <QObject>
#include <utp/localwindow.h>

using namespace utp;

class LocalWindowTest : public QObject
{
	Q_OBJECT
public:
	
private slots:
	void init()
	{
		memset(data,0xFF,13);
		memset(data2,0xEE,6);
	}
	
	void testWrite()
	{
		LocalWindow wnd(20);
		QVERIFY(wnd.maxWindow() == 20);
		
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.currentWindow() == 13);
		
		QVERIFY(wnd.write(data2,6) == 6);
		QVERIFY(wnd.currentWindow() == 19);
		
		QVERIFY(wnd.write(data2,6) == 1);
		QVERIFY(wnd.currentWindow() == 20);
	}
	
	void testRead()
	{
		LocalWindow wnd(20);
		QVERIFY(wnd.maxWindow() == 20);
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.write(data2,6) == 6);
		
		bt::Uint8 ret[19];
		QVERIFY(wnd.read(ret,19) == 19);
		QVERIFY(wnd.currentWindow() == 0);
		QVERIFY(memcmp(ret,data,13) == 0);
		QVERIFY(memcmp(ret+13,data2,6) == 0);
		
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.currentWindow() == 13);
		
		QVERIFY(wnd.write(data2,6) == 6);
		QVERIFY(wnd.currentWindow() == 19);
		
		QVERIFY(wnd.read(ret,19) == 19);
		QVERIFY(wnd.currentWindow() == 0);
		QVERIFY(memcmp(ret,data,13) == 0);
		QVERIFY(memcmp(ret+13,data2,6) == 0);
	}

private:
	bt::Uint8 data[13];
	bt::Uint8 data2[6];
};

QTEST_MAIN(LocalWindowTest)

#include "localwindowtest.moc"