/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#ifndef KT_PROPERTIESEXTENDER_H
#define KT_PROPERTIESEXTENDER_H

#include <QWidget>
#include <view/viewdelegate.h>
#include "ui_propertiesextender.h"


namespace bt
{
	class TorrentInterface;
}

namespace kt 
{
	/**
		Extender which shows properties about a torrent.
	*/
	class PropertiesExtender : public Extender,public Ui_PropertiesExtender
	{
		Q_OBJECT
	public:
		PropertiesExtender(bt::TorrentInterface* tc,QWidget* parent);
		virtual ~PropertiesExtender();
		
	public slots:
		void moveOnCompletionEnabled(bool on);
		void buttonClicked(QAbstractButton*);
		
	private:
		void apply();
	};

}

#endif // KT_PROPERTIESEXTENDER_H
