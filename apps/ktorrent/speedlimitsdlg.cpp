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
#include <qlabel.h>
#include <klocale.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <util/constants.h>
#include <interfaces/torrentinterface.h>

#include "speedlimitsdlg.h"
		
using namespace bt;
using namespace kt;

SpeedLimitsDlg::SpeedLimitsDlg(kt::TorrentInterface* ti,QWidget* parent, const char* name)
		: SpeedLimitsDlgBase(parent,name,true,0),tor(ti)
{
	m_main_caption->setText(i18n("Speed limits for <b>%1</b>:").arg(tor->getStats().torrent_name));
	Uint32 up,down;
	tor->getTrafficLimits(up,down);
	m_upload_rate->setValue(up / 1024);
	m_upload_rate->setMinValue(0);
	m_download_rate->setValue(down / 1024);
	m_download_rate->setMinValue(0);
	m_ok->setGuiItem(KStdGuiItem::ok());
	m_cancel->setGuiItem(KStdGuiItem::cancel());
	adjustSize();
}

SpeedLimitsDlg::~SpeedLimitsDlg()
{}


void SpeedLimitsDlg::accept()
{
	Uint32 up = m_upload_rate->value() * 1024;
	Uint32 down = m_download_rate->value() * 1024;
	tor->setTrafficLimits(up,down);
	QDialog::accept();
}



#include "speedlimitsdlg.moc"

