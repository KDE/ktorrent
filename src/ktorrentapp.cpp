/***************************************************************************
 *   Copyright (C) 2005 by Adam Treat                                      *
 *   treat@kde.org                                                         *
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

#include "ktorrentapp.h"

#include <kglobal.h>
#include <kstartupinfo.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <dcopclient.h>

#include <libutil/log.h>
#include <libtorrent/globals.h>
#include <libutil/functions.h>

#include "ktorrent.h"

KTorrentApp::KTorrentApp()
		: KUniqueApplication()
{}

KTorrentApp::~KTorrentApp()
{}

int KTorrentApp::newInstance()
{
	// register ourselves as a dcop client
	if (!dcopClient()->isRegistered() )
		dcopClient()->registerAs(name(), false);

	// see if we are starting with session management
	if (restoringSession())
	{
		RESTORE(KTorrent);
	}
	else
	{
		// no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		bt::Globals::instance().setDebugMode(args->isSet("debug"));

		QString data_dir = KGlobal::dirs()->saveLocation("data","ktorrent");
		if (!data_dir.endsWith(bt::DirSeparator()))
			data_dir += bt::DirSeparator();
		bt::Globals::instance().initLog(data_dir + "log");

		if (!mainWidget())
		{
			KTorrent *widget = new KTorrent();
			setMainWidget(widget);
			widget->show();
		}
		else
			KStartupInfo::setNewStartupId( mainWidget(), kapp->startupId());


		KTorrent *widget = ::qt_cast<KTorrent*>( mainWidget() );

		for (int i = 0; i < args->count(); i++)
		{
			widget->load(args->url(i));
		}

		args->clear();
	}
	return 0;
}

#include "ktorrentapp.moc"

