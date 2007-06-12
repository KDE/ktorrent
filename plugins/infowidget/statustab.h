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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef STATUSTAB_H
#define STATUSTAB_H

#include "statustabbase.h"
		
namespace kt
{
	class TorrentInterface;
	
	class StatusTab : public StatusTabBase
	{
		Q_OBJECT
	
	public:
		StatusTab(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
		virtual ~StatusTab();
		
	
	public slots:
		void changeTC(kt::TorrentInterface* tc);
		void update();
		void maxRatioReturnPressed();
    	void useLimitToggled(bool on);
		void useTimeLimitToggled(bool on);
		void timeValueChanged();
	private:
		void maxRatioUpdate();
	
	private:
		kt::TorrentInterface* curr_tc;
	};
}

#endif

