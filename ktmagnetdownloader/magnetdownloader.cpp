#include <stdio.h>
#include <stdlib.h>
#include <QCoreApplication>
#include <version.h>
#include <util/log.h>
#include "magnettest.h"
#include <KApplication>
#include <KLocalizedString>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

using namespace bt;


int main(int argc,char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr,"Usage: ktmagnetdownloader <magnet-link>\n");
		return 0;
	}
	
	bt::MagnetLink mlink(argv[1]);
	if (!mlink.isValid())
	{
		fprintf(stderr,"Invalid magnet link %s\n\n", argv[1]);
		fprintf(stderr,"Usage: ktmagnetdownloader <magnet-link>\n");
		return 0;
	}
	
	bt::SetClientInfo("ktmagnetdownloader",bt::MAJOR,bt::MINOR,bt::RELEASE,bt::BETA,"KT");
	bt::InitLog("ktmagnetdownload.log",false,true);
	bt::Log & log = Out();
	log.setOutputToConsole(true);
	log << "Downloading " << mlink.toString() << bt::endl;
	
	KAboutData about("ktmagnetdownloader", 0, ki18n("KTMagnetDownloader"),
					 "3.0dev", ki18n("KTorrent's magnet link downloader"),
					 KAboutData::License_GPL,
					 ki18n("(C) 2009 Joris Guisson"),
					 KLocalizedString(),
					 "http://www.ktorrent.org/");
	
	KCmdLineOptions options;
	options.add("+[Url]", ki18n("Document to open"));
	KCmdLineArgs::addCmdLineOptions(options);
	
	KCmdLineArgs::init(argc, argv,&about);

	KApplication app;
	MagnetTest mtest(mlink);
	return app.exec();
}
