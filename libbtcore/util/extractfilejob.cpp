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
#include <QThread>
#include "extractfilejob.h"
#include <QFile>

namespace bt
{
	class ExtractFileThread : public QThread
	{
	public:
		ExtractFileThread(QIODevice* in_dev,QIODevice* out_dev) : in_dev(in_dev),out_dev(out_dev),canceled(false)
		{
		}
		
		virtual ~ExtractFileThread()
		{
			delete in_dev;
			delete out_dev;
		}
		
		virtual void run()
		{
			char buf[4096];
			qint64 ret = 0;
			while ((ret = in_dev->read(buf,4096)) != 0 && !canceled)
			{
				out_dev->write(buf,ret);
			}
		}
		
		QIODevice* in_dev;
		QIODevice* out_dev;
		bool canceled;
	};
	
	ExtractFileJob::ExtractFileJob(KArchive* archive, const QString& path, const QString& dest) 
		: archive(archive),path(path),dest(dest),extract_thread(0)
	{
	}

	ExtractFileJob::~ExtractFileJob()
	{
		delete archive;
	}

	void ExtractFileJob::start()
	{
		// first find the file in the archive
		QStringList path_components = path.split("/",QString::SkipEmptyParts);
		const KArchiveDirectory* dir = archive->directory();
		for (int i = 0;i < path_components.count();i++)
		{
			// if we can't find it give back an error
			QString pc = path_components.at(i);
			if (!dir->entries().contains(pc))
			{
				setError(KIO::ERR_DOES_NOT_EXIST);
				emitResult();
				return;
			}
			
			const KArchiveEntry* e = dir->entry(pc);
			if (i < path_components.count() - 1)
			{
				// if we are not the last entry in the path, e must be a directory
				if (!e->isDirectory())
				{
					setError(KIO::ERR_DOES_NOT_EXIST);
					emitResult();
					return;
				}
				
				dir = (const KArchiveDirectory*)e;
			}
			else
			{
				// last in the path, must be a file
				if (!e->isFile())
				{
					setError(KIO::ERR_DOES_NOT_EXIST);
					emitResult();
					return;
				}
				
				// create a device to read the file and start a thread to do the reading
				KArchiveFile* file = (KArchiveFile*)e;
				QFile* out_dev = new QFile(dest);
				if (!out_dev->open(QIODevice::WriteOnly))
				{
					setError(KIO::ERR_CANNOT_OPEN_FOR_WRITING);
					emitResult();
					return;
				}
				
				QIODevice* in_dev = file->createDevice();
				extract_thread = new ExtractFileThread(in_dev,out_dev);
				connect(extract_thread,SIGNAL(finished()),this,SLOT(extractThreadDone()),Qt::QueuedConnection);
				extract_thread->start();
			}
		}
	}
	
	void ExtractFileJob::kill(bool quietly)
	{
		if (extract_thread)
		{
			extract_thread->canceled = true;
			extract_thread->wait();
			delete extract_thread;
			extract_thread = 0;
		}
		setError(KIO::ERR_USER_CANCELED);
		if (!quietly)
			emitResult();
	}

	void ExtractFileJob::extractThreadDone()
	{
		extract_thread->wait();
		delete extract_thread;
		extract_thread = 0;
		setError(0);
		emitResult();
	}

}

