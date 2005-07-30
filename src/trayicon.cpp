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
#include <kpopupmenu.h>
#include <kapplication.h>
#include "ktorrent.h"
#include "trayicon.h"
#include <qtooltip.h>

TrayIcon::TrayIcon(QWidget *parent, const char *name)
 : KSystemTray(parent, name)
{
	setPixmap(loadIcon("ktorrent"));
	connect(this,SIGNAL(quitSelected()),kapp,SLOT(quit()));
}

void TrayIcon::updateStats(const QString stats)
{
	QToolTip::add(this, "<b>KTorrent</b><br>"+stats);
}

TrayIcon::~TrayIcon()
{
}


#include "trayicon.moc"
