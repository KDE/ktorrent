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
#ifndef UPNPTESTAPP_H
#define UPNPTESTAPP_H

#include <kmainwindow.h>
#include <libktorrent/util/log.h>
#include <libktorrent/torrent/globals.h>
#include <plugins/upnp/upnprouter.h>
#include <plugins/upnp/upnpdescriptionparser.h>
#include <plugins/upnp/upnpmcastsocket.h>
#include <interfaces/logmonitorinterface.h>

class MainWidget;

using kt::UPnPRouter;

/**
	@author Joris Guisson <joris.guisson@gmail.com>
*/
class UPnPTestApp : public KMainWindow, public kt::LogMonitorInterface
{
	Q_OBJECT
public:
	UPnPTestApp(QWidget *parent = 0, const char *name = 0);
	virtual ~UPnPTestApp();
	
	virtual void message(const QString& line, unsigned int arg);
	bool queryExit();
	
private slots:
	void discovered(UPnPRouter* router);
	void onTestBtn();
	void onCloseBtn();
	
private:
	kt::UPnPMCastSocket* sock;
	MainWidget* mwnd;
};

#endif
