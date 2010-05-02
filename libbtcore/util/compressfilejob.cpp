/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include "compressfilejob.h"

#include <QFile>
#include <QThread>
#include <kio/global.h>
#include <kfilterdev.h>
#include "fileops.h"

namespace bt
{
	
	CompressThread::CompressThread(const QString & file) : file(file),canceled(false),err(0)
	{
	}
	
	CompressThread::~CompressThread()
	{}
		
	void CompressThread::run()
	{
		QFile in(file);
		
		// open input file readonly
		if (!in.open(QIODevice::ReadOnly))
		{
			err = KIO::ERR_CANNOT_OPEN_FOR_READING;
			printf("CompressThread: failed to open input file %s for reading: %s\n",in.fileName().toLocal8Bit().constData(),in.errorString().toLocal8Bit().constData());
			return;
		}
		
		// open output file 
		QIODevice* dev = KFilterDev::deviceForFile(file + ".gz","application/x-gzip");
		if (!dev || !dev->open(QIODevice::WriteOnly))
		{
			err = KIO::ERR_CANNOT_OPEN_FOR_WRITING;
			printf("CompressThread: failed to open out file for writing");
			return;
		}
		
		// copy the data
		char buf[4096];
		while (!canceled && !in.atEnd())
		{
			int len = in.read(buf,4096);
			if (len == 0)
				break;
			
			dev->write(buf,len);
		}
		
		delete dev;
		in.close();
		if (canceled)
		{
			// delete output file when canceled
			bt::Delete(file + ".gz",true);
		}
		else
		{
			// delete the input file upon success
			bt::Delete(file,true);
		}
	}
	
	void CompressThread::cancel()
	{
		canceled = true;
	}
	
	////////////////////////////////////////////////////////////

	CompressFileJob::CompressFileJob(const QString & file) : file(file),compress_thread(0)
	{
	}

	CompressFileJob::~CompressFileJob()
	{
	}
	
	void CompressFileJob::start()
	{
		compress_thread = new CompressThread(file);
		connect(compress_thread,SIGNAL(finished()),this,SLOT(compressThreadFinished()),Qt::QueuedConnection);
		compress_thread->start();
	}
	
	void CompressFileJob::kill(bool quietly)
	{
		if (compress_thread)
		{
			compress_thread->cancel();
			compress_thread->wait();
			delete compress_thread;
			compress_thread = 0;
		}
		setError(KIO::ERR_USER_CANCELED);
		if (!quietly)
			emitResult();
	}
	
	void CompressFileJob::compressThreadFinished()
	{
		setError(compress_thread->error());
		compress_thread->wait();
		delete compress_thread;
		compress_thread = 0;
		emitResult();
	}


}
