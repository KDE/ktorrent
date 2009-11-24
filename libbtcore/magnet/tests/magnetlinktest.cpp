#include <QtTest>
#include <QObject>
#include <magnet/magnetlink.h>
#include <util/log.h>

class MagnetLinkTest : public QObject
{
	Q_OBJECT
public:
	MagnetLinkTest() {}
	virtual ~MagnetLinkTest() {}

private slots:
	void init()
	{
	}
	
	void testParsing()
	{
		QString data = "magnet:?xt=urn:btih:fe377e017ef52efa83251231b5b991ffae0e77ae&dn=Indie+Top+50+-+Best+of+Indie";
		bt::MagnetLink mlink(data);
		QVERIFY(mlink.isValid());
		QCOMPARE(mlink.displayName(),QString("Indie Top 50 - Best of Indie"));
		
		bt::Uint8 hash[] = {
			0xfe, 0x37, 0x7e, 0x01, 0x7e, 
			0xf5, 0x2e, 0xfa, 0x83, 0x25, 
			0x12, 0x31, 0xb5, 0xb9, 0x91, 
			0xff, 0xae, 0x0e, 0x77, 0xae 
		};
		//printf("hash = %s\n",mlink.infoHash().toString().toAscii().constData());
		QCOMPARE(mlink.infoHash(),bt::SHA1Hash(hash));
	}
	
	void testInvalidUrl()
	{
		QStringList invalid;
		invalid << "dinges:";
		invalid << "magnet:?xt=dinges";
		invalid << "magnet:?xt=urn:btih:fe377e017ef52ef";
		invalid << "magnet:?xt=urn:btih:fe377e017ef52efa83251231b5b991ffae0e77--";
		
		foreach (const QString & data,invalid)
		{
			bt::MagnetLink mlink(data);
			QVERIFY(!mlink.isValid());
		}
	}
};

QTEST_MAIN(MagnetLinkTest)

#include "magnetlinktest.moc"