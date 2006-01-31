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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
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
#include <util/error.h>
#include <util/object.h>
#include <torrent/globals.h>

using namespace bt;

/*
void StupidWarningMessagesFromQt( QtMsgType type, const char *msg )
{
	switch ( type ) {
		case QtDebugMsg:
			fprintf( stderr, "Debug: %s\n", msg );
			break;
		case QtWarningMsg:
			fprintf( stderr, "Warning: %s\n", msg );
			break;
		case QtFatalMsg:
			fprintf( stderr, "Fatal: %s\n", msg );
			abort();                    // deliberately core dump
	}
}
*/

static const char description[] =
    I18N_NOOP("A BitTorrent program for KDE");

static const char version[] = "1.3dev";

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
//	qInstallMsgHandler( StupidWarningMessagesFromQt );
	KAboutData about("ktorrent", I18N_NOOP("KTorrent"), version, description,
					 KAboutData::License_GPL, "(C) 2005 Joris Guisson", 0,
					 "http://ktorrent.pwsp.net/");
	about.addAuthor("Joris Guisson", 0, "joris.guisson@gmail.com" );
	about.addAuthor("Ivan Vasic",0,"ivasic@gmail.com");
	
	about.addCredit("The-Error",I18N_NOOP("The downloads icon"),"zotrix@eunet.yu");
	about.addCredit("Adam Treat", 0, "treat@kde.org" );
	about.addCredit("Danny Allen",
					I18N_NOOP("1.0 application icon"),
					"danny@dannyallen.co.uk");
	about.addCredit("Vincent Wagelaar",0,"vincent@ricardis.tudelft.nl");
	about.addCredit("Knut Morten Johansson",0,"knut@johansson.com");
	about.addCredit("Mladen Babic",
					I18N_NOOP("1.1 application icon and a couple of others"),"bmladen@EUnet.yu");
	about.addCredit("Felix Berger",
					I18N_NOOP("ChunkBar's tooltip and IWFileTreeItem sorting"),
					"bflat1@gmx.net");
	about.addCredit("Andreas Kling",0,"kling@impul.se");
	about.addCredit("Felipe Sateler",0,"fsateler@gmail.com");
	
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);
	
	KTorrentApp::addCmdLineOptions();
	if (!KTorrentApp::start())
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


