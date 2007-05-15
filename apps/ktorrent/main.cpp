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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include <stdlib.h>

#include "ktorrentapp.h"

#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <fcntl.h>
#include <util/error.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <util/fileops.h>
#include <ktversion.h>
#include <functions.h>
#include <qfile.h>
#include <qdir.h>

using namespace bt;



void StupidWarningMessagesFromQt( QtMsgType type, const char *msg )
{
	switch ( type ) 
	{
		case QtDebugMsg:
		//	printf("Qt: Debug: %s\n",msg);
			break;
		case QtWarningMsg:
			printf("Qt: Warning: %s\n",msg);
			break;
		case QtFatalMsg:
			printf("Qt: Fatal : %s\n",msg);
			abort();                    // deliberately core dump
			break;
	}
}



static const char description[] =
    I18N_NOOP("A BitTorrent program for KDE");


bool GrabPIDLock()
{
	// open the PID file in the users ktorrent directory and attempt to lock it
	QString pid_file = QDir::homeDirPath() + "/.ktorrent.lock";
		
	int fd = open(QFile::encodeName(pid_file),O_RDWR|O_CREAT,0640);
	if (fd < 0)
	{
		fprintf(stderr,"Failed to open KT lock file %s : %s\n",pid_file.ascii(),strerror(errno));
		return false;
	}

	if (lockf(fd,F_TLOCK,0)<0) 
	{
		fprintf(stderr,"Failed to get lock on %s : %s\n",pid_file.ascii(),strerror(errno));
		return false;
	}
		
	char str[20];
	sprintf(str,"%d\n",getpid());
	write(fd,str,strlen(str)); /* record pid to lockfile */

	// leave file open, so nobody else can lock it until KT exists
	return true;
}


static KCmdLineOptions options[] =
{
	{ "debug", I18N_NOOP("Debug mode"), 0 },
	{ "+[URL]", I18N_NOOP( "Document to open" ), 0 },
	KCmdLineLastOption
};

int main(int argc, char **argv)
{
	// ignore SIGPIPE's
	signal(SIGPIPE,SIG_IGN);
	qInstallMsgHandler( StupidWarningMessagesFromQt );
	KAboutData about("ktorrent", I18N_NOOP("KTorrent"), kt::VERSION_STRING, description,
					 KAboutData::License_GPL, "(C) 2005 -2006 Joris Guisson and Ivan Vasic", 0,
					 "http://www.ktorrent.org/");
	about.addAuthor("Joris Guisson", 0, "joris.guisson@gmail.com" );
	about.addAuthor("Ivan Vasic",0,"ivasic@gmail.com");
	about.addAuthor("Alan Jones",0,"skyphyr@gmail.com");
	about.addAuthor("Diego R. Brogna",0,"dierbro@gmail.com");

	about.addCredit("Mladen Babic",
					I18N_NOOP("Application icon and a couple of others"),"bmladen@EUnet.yu");
		about.addCredit("The-Error",I18N_NOOP("The downloads icon"),"zotrix@eunet.yu");
	about.addCredit("Adam Treat", 0, "treat@kde.org" );
	about.addCredit("Danny Allen",
					I18N_NOOP("1.0 application icon"),
					"danny@dannyallen.co.uk");
	about.addCredit("Vincent Wagelaar",0,"vincent@ricardis.tudelft.nl");
	about.addCredit("Knut Morten Johansson",0,"knut@johansson.com");
	about.addCredit("Felix Berger",
					I18N_NOOP("ChunkBar's tooltip and IWFileTreeItem sorting"),
					"bflat1@gmx.net");
	about.addCredit("Andreas Kling",0,"kling@impul.se");
	about.addCredit("Felipe Sateler",0,"fsateler@gmail.com");
	about.addCredit("Maxmind", I18N_NOOP("Country locator for InfoWidget plugin. Flags are taken from http://flags.blogpotato.de/ so thanks to them too."),0, "http://www.maxmind.com/");
	about.addCredit("Adam Forsyth",I18N_NOOP("File prioritization"),"agforsyth@gmail.com");
	about.addCredit("Thomas Bernard",I18N_NOOP("Miniupnp was used as an example for our own UPnP implementation"),0,"http://miniupnp.free.fr/");
	about.addCredit("Diego Rosario Brogna",I18N_NOOP("Global max share ratio patch"),0,"dierbro@gmail.com");
	about.addCredit("Lesly Weyts",I18N_NOOP("Zeroconf enhancements"),0,0);
	about.addCredit("Kevin Andre",I18N_NOOP("Zeroconf enhancements"),0,"http://users.telenet.be/hyperquantum");
	about.addCredit("Dagur Valberg Johannsson",I18N_NOOP("Coldmilk webgui"),"dagurval@pvv.ntnu.no");
	about.addCredit("Alexander Dymo",I18N_NOOP("IDEAl code from KDevelop"),"adymo@kdevelop.org");
	about.addCredit("Scott Wolchok",I18N_NOOP("Conversion speed improvement in ipfilter plugin"),"swolchok@umich.edu");
	about.addCredit("Bryan Burns of Juniper Networks",I18N_NOOP("Discovered 2 security vulnerabilities (both are fixed)"),0);
	about.addCredit("Goten Xiao",I18N_NOOP("Patch to load silently with a save location"),0);
	about.addCredit("Rapsys",I18N_NOOP("Fixes in PHP code of webinterface"),0);
	about.addCredit("Athantor",I18N_NOOP("XFS specific disk preallocation"),0);
	about.addCredit("twisted_fall",I18N_NOOP("Patch to not show very low speeds"),"twisted.fall@gmail.com");

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);
	
	KTorrentApp::addCmdLineOptions();
	if (!KTorrentApp::start())
	{
		fprintf(stderr, "ktorrent is already running!\n");
		return 0;
	}

	// need to grab lock after the fork call in start, otherwise this will not work properly
	if (!GrabPIDLock())
	{
		fprintf(stderr, "ktorrent is already running!\n");
		return 0;
	}
	
	try
	{
		KTorrentApp app;
		app.exec();
	}
	catch (bt::Error & e)
	{
		fprintf(stderr, "Aborted by error : %s\n",e.toString().ascii());
	}
	Globals::cleanup();
	
//	printf("\n\nObjects alive = %i\n\n",(unsigned int)Object::numAlive());
	return 0;
}


