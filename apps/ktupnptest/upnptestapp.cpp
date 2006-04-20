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
#include <kpushbutton.h>
#include <klocale.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <qtextbrowser.h>
#include <util/error.h>
#include "upnptestapp.h"
#include "mainwidget.h"

using namespace bt;
using namespace kt;

UPnPTestApp::UPnPTestApp(QWidget *parent, const char *name)
		: KMainWindow(parent, name)
{
	sock = new UPnPMCastSocket(true);
	connect(sock,SIGNAL(discovered( UPnPRouter* )),this,SLOT(discovered( UPnPRouter* )));
	mwnd = new MainWidget(this);
	setCentralWidget(mwnd);
	connect(mwnd->test_btn,SIGNAL(clicked()),this,SLOT(onTestBtn()));
	connect(mwnd->close_btn,SIGNAL(clicked()),this,SLOT(onCloseBtn()));
	bt::Log & lg = Out();
	lg.addMonitor(this);
	Out() << "UPnPTestApp started up !" << endl;
}


UPnPTestApp::~UPnPTestApp()
{
	bt::Log & lg = Out();
	lg.removeMonitor(this);
	delete sock;
}

void UPnPTestApp::discovered(kt::UPnPRouter* router)
{
	try
	{
		router->forward(9999,UPnPRouter::TCP);
	}
	catch (Error & e)
	{
		KMessageBox::error(this,e.toString());
	}
}

void UPnPTestApp::onTestBtn()
{
	sock->discover();
}

void UPnPTestApp::onCloseBtn()
{
	kapp->quit();
}

void UPnPTestApp::message(const QString& line)
{
	mwnd->output->append(line);
}


#include "upnptestapp.moc"
