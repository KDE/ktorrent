/***************************************************************************
 *   Copyright (C) 2009 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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
#include "scanlistener.h"
#include "scanextender.h"

namespace kt
{
	ScanListener::ScanListener(bt::TorrentInterface* tc) : QObject(tc),bt::DataCheckerListener(false),done(false),tc(tc)
	{
		num_chunks = 0;
		total_chunks = 0;
		num_downloaded = 0;
		num_failed = 0;
		num_found = 0;
		num_not_downloaded = 0;
	}
	
	
	ScanListener::~ScanListener()
	{
		
	}
	
	void ScanListener::progress(bt::Uint32 num, bt::Uint32 total)
	{
		QMutexLocker lock(&mutex);
		num_chunks = num;
		total_chunks = total;
	}
	
	void ScanListener::status(bt::Uint32 num_failed, bt::Uint32 num_found, bt::Uint32 num_downloaded, bt::Uint32 num_not_downloaded)
	{
		QMutexLocker lock(&mutex);
		this->num_downloaded = num_downloaded;
		this->num_failed = num_failed;
		this->num_found = num_found;
		this->num_not_downloaded = num_not_downloaded;
	}

	void ScanListener::restart()
	{
		num_chunks = 0;
		total_chunks = 0;
		num_downloaded = 0;
		num_failed = 0;
		num_found = 0;
		num_not_downloaded = 0;
		done = false;
		restarted();
		tc->startDataCheck(this);
	}
	
	void ScanListener::finished()
	{
		done = true;
		scanFinished();
	}
	
	
	void ScanListener::emitCloseRequested()
	{
		emit closeRequested(this);
	}


	QWidget* ScanListener::createExtender() 
	{
		return new ScanExtender(this,tc,0);
	}

	
}

