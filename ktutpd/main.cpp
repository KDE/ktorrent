#include <QCoreApplication>
#include "utpdaemon.h"

int main(int argc,char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr,"Usage: ktutpd <listen-port>\n");
		return -1;
	}
	
	quint16 port = atoi(argv[1]);
	QCoreApplication app(argc,argv);
	utp::UTPDaemon daemon(port);
	if (!daemon.start())
		return -1;
	else
		return app.exec();
}