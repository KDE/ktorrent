/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <fcntl.h>
#include <exception>
#include <QDir>
#include <QFile>
#include <kurl.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include "app.h"
#include "version.h"
#include "ktversion.h"
#include <kdebug.h>
#include <torrent/globals.h>
#include <util/error.h>
#include <util/log.h>
#include <util/functions.h>

using namespace bt;

#ifndef Q_WS_WIN
bool GrabPIDLock()
{
	// open the PID file in the /tmp directory and attempt to lock it
	QString pid_file = QString(QDir::tempPath()+"/.ktorrent_kde4_%1.lock").arg(getuid());

	int fd = open(QFile::encodeName(pid_file),O_RDWR|O_CREAT,0640);
	if (fd < 0)
	{
		fprintf(stderr,"Failed to open KT lock file %s : %s\n",pid_file.toAscii().constData(),strerror(errno));
		return false;
	}

	if (lockf(fd,F_TLOCK,0)<0) 
	{
		fprintf(stderr,"Failed to get lock on %s : %s\n",pid_file.toAscii().constData(),strerror(errno));
		return false;
	}
		
	char str[20];
	sprintf(str,"%d\n",getpid());
	write(fd,str,strlen(str)); /* record pid to lockfile */

	// leave file open, so nobody else can lock it until KT exists
	return true;
}
#endif


int main(int argc, char **argv)
{	
#ifndef Q_WS_WIN
	// ignore SIGPIPE and SIGXFSZ
	signal(SIGPIPE,SIG_IGN);
	signal(SIGXFSZ,SIG_IGN);
#endif
	
	if (!bt::InitLibKTorrent())
	{
		fprintf(stderr,"Failed to initialize libktorrent\n");
		return -1;
	}

	bt::SetClientInfo("KTorrent",kt::MAJOR,kt::MINOR,kt::RELEASE,kt::VERSION_TYPE,"KT");
	
	KAboutData about(
		"ktorrent", 0, ki18n("KTorrent"),
		kt::VERSION_STRING, ki18n("Bittorrent client for KDE"),
		KAboutData::License_GPL, 
		ki18n("(C) 2005 - 2010 Joris Guisson and Ivan Vasic"), 
		KLocalizedString(),
		"http://www.ktorrent.org/");

	about.addAuthor(ki18n("Joris Guisson"), KLocalizedString(), "joris.guisson@gmail.com","http://ktorrent.org" );
	about.addAuthor(ki18n("Ivan Vasic"),KLocalizedString(),"ivasic@gmail.com");
	about.addAuthor(ki18n("Alan Jones"),ki18n("BitFinder Plugin"),"skyphyr@gmail.com");
	about.addAuthor(ki18n("Diego R. Brogna"),ki18n("Webinterface Plugin"),"dierbro@gmail.com");
	about.addAuthor(ki18n("Krzysztof Kundzicz"),ki18n("Statistics Plugin"),"athantor@gmail.com"); 

	about.addCredit(ki18n("Mladen Babic"),	ki18n("Application icon and a couple of others"),"bmladen@EUnet.yu");
	about.addCredit(ki18n("Adam Treat"), KLocalizedString(), "treat@kde.org" );
	about.addCredit(ki18n("Danny Allen"),ki18n("1.0 application icon"),"danny@dannyallen.co.uk");
	about.addCredit(ki18n("Vincent Wagelaar"),KLocalizedString(),"vincent@ricardis.tudelft.nl");
	about.addCredit(ki18n("Knut Morten Johansson"),KLocalizedString(),"knut@johansson.com");
	about.addCredit(ki18n("Felix Berger"),	ki18n("ChunkBar's tooltip and IWFileTreeItem sorting"),"bflat1@gmx.net");
	about.addCredit(ki18n("Andreas Kling"),KLocalizedString(),"kling@impul.se");
	about.addCredit(ki18n("Felipe Sateler"),KLocalizedString(),"fsateler@gmail.com");
	about.addCredit(ki18n("Maxmind"), ki18n("Country locator for InfoWidget plugin. Flags are taken from http://flags.blogpotato.de/ so thanks to them too."),0, "http://www.maxmind.com/");
	about.addCredit(ki18n("Adam Forsyth"),ki18n("File prioritization and some other patches"),"agforsyth@gmail.com");
	about.addCredit(ki18n("Thomas Bernard"),ki18n("Miniupnp was used as an example for our own UPnP implementation"),0,"http://miniupnp.free.fr/");
	about.addCredit(ki18n("Diego Rosario Brogna"),ki18n("Global max share ratio patch"),0,"dierbro@gmail.com");
	about.addCredit(ki18n("Lesly Weyts"),ki18n("Zeroconf enhancements"),0,0);
	about.addCredit(ki18n("Kevin Andre"),ki18n("Zeroconf enhancements"),0,"http://users.edpnet.be/hyperquantum/");
	about.addCredit(ki18n("Dagur Valberg Johannsson"),ki18n("Coldmilk webgui"),"dagurval@pvv.ntnu.no");
	about.addCredit(ki18n("Alexander Dymo"),ki18n("IDEAl code from KDevelop"),"adymo@kdevelop.org");
	about.addCredit(ki18n("Scott Wolchok"),ki18n("Conversion speed improvement in ipfilter plugin"),"swolchok@umich.edu");
	about.addCredit(ki18n("Bryan Burns of Juniper Networks"),ki18n("Discovered 2 security vulnerabilities (both are fixed)"),0);
	about.addCredit(ki18n("Goten Xiao"),ki18n("Patch to load silently with a save location"),0);
	about.addCredit(ki18n("Rapsys"),ki18n("Fixes in PHP code of webinterface"),0);
	about.addCredit(ki18n("Athantor"),ki18n("XFS specific disk preallocation"),0);
	about.addCredit(ki18n("twisted_fall"),ki18n("Patch to not show very low speeds"),"twisted.fall@gmail.com");
	about.addCredit(ki18n("Lucke"),ki18n("Patch to show potentially firewalled status"),0);
	about.addCredit(ki18n("Modestas Vainius"),ki18n("Several patches"),"modestas@vainius.eu");
	about.addCredit(ki18n("Stefan Monov"),ki18n("Patch to hide menu bar"),"logixoul@gmail.com");
	about.addCredit(ki18n("The_Kernel"),ki18n("Patch to change file priorities in the webgui"),"kernja@cs.earlham.edu");
	about.addCredit(ki18n("Rafał Miłecki"),ki18n("Several webgui patches"),"zajec5@gmail.com");
	about.addCredit(ki18n("Ozzi"),ki18n("Fixes for several warnings"),"ossi@masiina.no-ip.info");
	about.addCredit(ki18n("Markus Brueffer"),ki18n("Patch to fix free diskspace calculation on FreeBSD"),"markus@brueffer.de");
	about.addCredit(ki18n("Lukas Appelhans"),ki18n("Patch to fix a crash in ChunkDownloadView"),"l.appelhans@gmx.de");
	about.addCredit(ki18n("Rickard Närström"),ki18n("A couple of bugfixes"),"rickard.narstrom@gmail.com");
	about.addCredit(ki18n("caruccio"),ki18n("Patch to load torrents silently from the command line"),"mateus@caruccio.com");
	about.addCredit(ki18n("Lee Olson"),ki18n("New set of icons"),"leetolson@gmail.com");
	about.addCredit(ki18n("Aaron J. Seigo"),ki18n("Drag and drop support for Plasma applet"),"aseigo@kde.org");
	about.addCredit(ki18n("Ian Higginson"),ki18n("Patch to cleanup the plugin list"),"xeriouxi@fastmail.fm");
	about.addCredit(ki18n("Amichai Rothman"),ki18n("Patch to make the Plasma applet a popup applet"),"amichai@amichais.net");
	about.addCredit(ki18n("Leo Trubach"),ki18n("Patch to add support for IP ranges in IP filter dialog"),"leotrubach@gmail.com");
	about.addCredit(ki18n("Andrei Barbu"),ki18n("Feature which adds the date a torrent was added"),"andrei@0xab.com");
	KCmdLineArgs::init(argc, argv, &about);
	about.addCredit(ki18n("Jonas Lundqvist"),ki18n("Feature to disable authentication in the webinterface"),"jonas@gannon.se"); 
	about.addCredit(ki18n("Jaroslaw Swierczynski"),ki18n("Exclusion patterns in the syndication plugin"),"swiergot@gmail.com");

	KCmdLineOptions options;
	options.add("+[Url]", ki18n("Document to open"));
	options.add("silent", ki18n("Silently open torrent given on URL"));
	KCmdLineArgs::addCmdLineOptions(options);
	
	kt::App::addCmdLineOptions();
	if (!kt::App::start())
	{
		fprintf(stderr, "ktorrent is already running !\n");
		return 0;
	}

#ifndef Q_WS_WIN	
	// need to grab lock after the fork call in start, otherwise this will not work properly
	if (!GrabPIDLock())
	{
		fprintf(stderr, "ktorrent is already running !\n");
		return 0;

	}
#endif

	try
	{
		kt::App app;
		app.setQuitOnLastWindowClosed(false);
		app.exec();
	}
	catch (bt::Error & err)
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Uncaught exception: " << err.toString() << endl;
	}
	catch (std::exception & err)
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Uncaught exception: " << err.what() << endl;
	}
	catch (...)
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Uncaught unknown exception " << endl;
	}
	bt::Globals::cleanup();
	return 0;
}



