/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#include <QFile>
#include <KFilterDev>
#include <util/log.h>
#include <util/fileops.h>
#include "decompressfilejob.h"
#include <kmimetype.h>

namespace bt
{
	DecompressThread::DecompressThread(const QString & file,const QString & dest_file) 
	: file(file),dest_file(dest_file),canceled(false),err(0)
	{
	}
	
	DecompressThread::~DecompressThread()
	{}
	
	void DecompressThread::run()
	{
		QFile out(dest_file);
		
		// open input file readonly
		if (!out.open(QIODevice::WriteOnly))
		{
			err = KIO::ERR_CANNOT_OPEN_FOR_WRITING;
			Out(SYS_GEN|LOG_NOTICE) << "Failed to open " << dest_file << " : " << out.errorString() << endl;
			return;
		}
		
		// open output file 
		QString mime = KMimeType::findByPath(file)->name();
		QIODevice* dev = KFilterDev::deviceForFile(file,mime);
		if (!dev || !dev->open(QIODevice::ReadOnly))
		{
			err = KIO::ERR_CANNOT_OPEN_FOR_READING;
			if (dev)
				Out(SYS_GEN|LOG_NOTICE) << "Failed to open " << file << " : " << dev->errorString() << endl;
			else
				Out(SYS_GEN|LOG_NOTICE) << "Failed to open " << file << endl;
			return;
		}
		
		// copy the data
		char buf[4096];
		while (!canceled && !dev->atEnd())
		{
			int len = dev->read(buf,4096);
			if (len == 0)
				break;
			
			out.write(buf,len);
		}
		
		delete dev;
		out.close();
		if (canceled)
		{
			// delete output file when canceled
			bt::Delete(dest_file,true);
		}
		else
		{
			// delete the input file upon success
			bt::Delete(file,true);
		}
	}
	
	void DecompressThread::cancel()
	{
		canceled = true;
	}
	
	///////////////////////////////////////////
	
	
	DecompressFileJob::DecompressFileJob(const QString& file, const QString& dest) 
		: file(file),dest(dest),decompress_thread(0)
	{
	}

	
	DecompressFileJob::~DecompressFileJob()
	{
	}

	void DecompressFileJob::start()
	{
		decompress_thread = new DecompressThread(file,dest);
		connect(decompress_thread,SIGNAL(finished()),this,SLOT(decompressThreadFinished()),Qt::QueuedConnection);
		decompress_thread->start();
	}
	
	void DecompressFileJob::kill(bool quietly)
	{
		if (decompress_thread)
		{
			decompress_thread->cancel();
			decompress_thread->wait();
			delete decompress_thread;
			decompress_thread = 0;
		}
		setError(KIO::ERR_USER_CANCELED);
		if (!quietly)
			emitResult();
	}
	
	void DecompressFileJob::decompressThreadFinished()
	{
		setError(decompress_thread->error());
		decompress_thread->wait();
		delete decompress_thread;
		decompress_thread = 0;
		emitResult();
	}

}
