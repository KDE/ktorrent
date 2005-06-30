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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <klocale.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include <stdlib.h>

#include "ktorrentapp.h"


static const char description[] =
    I18N_NOOP("A BitTorrent program for KDE");

static const char version[] = "1.0";

static KCmdLineOptions options[] =
{
	{ "debug", I18N_NOOP("Debug mode"), 0 },
	{ "+[URL]", I18N_NOOP( "Document to open" ), 0 },
	KCmdLineLastOption
};

int main(int argc, char **argv)
{	
	KAboutData about("ktorrent", I18N_NOOP("KTorrent"), version, description,
	                 KAboutData::License_GPL, "(C) 2005 Joris Guisson", 0, 0, "joris.guisson@gmail.com");
	about.addAuthor( "Joris Guisson", 0, "joris.guisson@gmail.com" );
	about.addCredit("The-Error","The downloads icon","zotrix@eunet.yu");
	about.addCredit( "Adam Treat", 0, "treat@kde.org" );
	about.addCredit("Danny Allen","Application icon","danny@dannyallen.co.uk");
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);
	
	KTorrentApp::addCmdLineOptions();
	if (!KTorrentApp::start())
	{
		fprintf(stderr, "ktorrent is already running!\n");
		return 0;
	}
	
	KTorrentApp app;
	return app.exec();
}
