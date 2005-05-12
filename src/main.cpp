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


#include <kapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <libtorrent/log.h>
#include <libtorrent/globals.h>


#include "ktorrent.h"


static const char description[] =
    I18N_NOOP("A BitTorrent program for KDE");

static const char version[] = "1.0rc1";

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
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication app;

	// register ourselves as a dcop client
	app.dcopClient()->registerAs(app.name(), false);

	// see if we are starting with session management
	if (app.isRestored())
	{
		RESTORE(KTorrent);
	}
	else
	{
		// no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		bt::Globals::instance().setDebugMode(args->isSet("debug"));

		QString data_dir = KGlobal::dirs()->saveLocation("data","ktorrent");
		if (!data_dir.endsWith("/"))
			data_dir += "/";
		bt::Globals::instance().initLog(data_dir + "log");
		
		KTorrent *widget = new KTorrent();
		widget->show();

		
		
		for (int i = 0; i < args->count(); i++)
		{
			widget->load(args->url(i));
		}
		
		args->clear();
	}

	return app.exec();
}
