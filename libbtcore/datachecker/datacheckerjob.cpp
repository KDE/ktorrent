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

#include "datacheckerjob.h"
#include "datacheckerlistener.h"
#include "datacheckerthread.h"
#include "multidatachecker.h"
#include "singledatachecker.h"
#include <torrent/torrentcontrol.h>
#include <util/functions.h>

namespace bt
{
	
	DataCheckerJob::DataCheckerJob(bt::DataCheckerListener* lst, bt::TorrentControl* tc): Job(true,tc),listener(lst)
	{
		dcheck_thread = 0;
		killed = false;
	}
	
	
	DataCheckerJob::~DataCheckerJob()
	{
	}

	void DataCheckerJob::start()
	{
		DataChecker* dc = 0;
		const TorrentStats & stats = torrent()->getStats();
		if (stats.multi_file_torrent)
			dc = new MultiDataChecker();
		else
			dc = new SingleDataChecker();
		
		dc->setListener(listener);
		TorrentControl* tor = torrent();
		dcheck_thread = new DataCheckerThread(
				dc,tor->downloadedChunksBitSet(),
				stats.output_path,tor->getTorrent(),
				tor->getTorDir() + "dnd" + bt::DirSeparator());
				
		connect(dcheck_thread,SIGNAL(finished()),this,SLOT(finished()),Qt::QueuedConnection);
		
		torrent()->beforeDataCheck();
		
		// dc->check(stats.output_path,*tor,tordir + "dnd" + bt::DirSeparator());
		dcheck_thread->start(QThread::IdlePriority);
	}

	void DataCheckerJob::kill(bool quietly)
	{
		killed = true;
		if (dcheck_thread && dcheck_thread->isRunning())
		{
			listener->stop();
			dcheck_thread->wait();
			dcheck_thread->deleteLater();
			dcheck_thread = 0;
		}
		bt::Job::kill(quietly);
	}
	
	
	void DataCheckerJob::finished()
	{
		if (!killed)
		{
			DataChecker* dc = dcheck_thread->getDataChecker();
			torrent()->afterDataCheck(listener,dc->getResult(),dcheck_thread->getError());
		}
		dcheck_thread->deleteLater();
		dcheck_thread = 0;
		setError(0);
		emitResult();
	}


}