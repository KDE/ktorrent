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

#ifndef INFOWIDGET_H
#define INFOWIDGET_H


#include "infowidgetbase.h"

class KTorrentMonitor;

namespace bt
{
	class TorrentControl;
}

class InfoWidget : public InfoWidgetBase
{
	Q_OBJECT

public:
	InfoWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
	virtual ~InfoWidget();
	

public slots:
	void changeTC(bt::TorrentControl* tc);
	void update();

private:
	void fillFileTree();
	
private:
	KTorrentMonitor* monitor;
	bt::TorrentControl* curr_tc;
};

#endif

