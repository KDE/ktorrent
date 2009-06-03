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

#ifndef DECOMPRESSFILEJOB_H
#define DECOMPRESSFILEJOB_H

#include <QThread>
#include <kio/job.h>
#include <btcore_export.h>

namespace bt
{
	
	/**
	* Thread which decompresses a single file
	*/
	class BTCORE_EXPORT DecompressThread : public QThread
	{
	public:
		DecompressThread(const QString & file,const QString & dest_file);
		virtual ~DecompressThread();
		
		/// Run the decompression thread
		virtual void run();
		
		/// Cancel the thread, things should be cleaned up properly
		void cancel();
		
		/// Get the error which happened (0 means no error)
		int error() const {return err;}
		
	private:
		QString file;
		QString dest_file;
		bool canceled;
		int err;
	};

	/**
		Decompress a file and remove it when completed successfully.
	*/
	class BTCORE_EXPORT DecompressFileJob : public KIO::Job
	{
		Q_OBJECT
	public:
		DecompressFileJob(const QString & file,const QString & dest);
		virtual ~DecompressFileJob();
		
		virtual void start();
		virtual void kill(bool quietly=true);
		
	private slots:
		void decompressThreadFinished();
		
	private:
		QString file;
		QString dest;
		DecompressThread* decompress_thread;
	};
}

#endif // DECOMPRESSFILEJOB_H
