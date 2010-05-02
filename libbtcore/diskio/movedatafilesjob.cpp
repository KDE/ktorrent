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
#include "movedatafilesjob.h"
#include <QFileInfo>
#include <kio/jobuidelegate.h>
#include <util/log.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <interfaces/torrentfileinterface.h>

namespace bt
{

	MoveDataFilesJob::MoveDataFilesJob() : Job(true,0),err(false),active_job(0),running_recovery_jobs(0)
	{}

	
	MoveDataFilesJob::MoveDataFilesJob(const QMap< TorrentFileInterface*, QString >& fmap) 
		: Job(true,0),err(false),active_job(0),running_recovery_jobs(0)
	{
		file_map = fmap;
		QMap<TorrentFileInterface*,QString>::const_iterator i = file_map.constBegin();
		while (i != file_map.constEnd())
		{
			TorrentFileInterface* tf = i.key();
			QString dest = i.value();
			if (QFileInfo(dest).isDir())
			{
				QString path = tf->getUserModifiedPath();
				if (!dest.endsWith(bt::DirSeparator()))
					dest += bt::DirSeparator();
				
				int last = path.lastIndexOf(bt::DirSeparator());
				QString dst = dest + path.mid(last+1);
				if (QFileInfo(tf->getPathOnDisk()).canonicalPath() != QFileInfo(dst).canonicalPath())
					addMove(tf->getPathOnDisk(),dst);
			}
			else if (QFileInfo(tf->getPathOnDisk()).canonicalPath() != QFileInfo(i.value()).canonicalPath())
			{
				addMove(tf->getPathOnDisk(),i.value());
			}
			i++;
		}
	}


	MoveDataFilesJob::~MoveDataFilesJob()
	{}

	void MoveDataFilesJob::addMove(const QString & src,const QString & dst)
	{
		todo.insert(src,dst);
	}
		
	void MoveDataFilesJob::onJobDone(KJob* j)
	{
		if (j->error() || err)
		{
			if (!err)
				setError(KIO::ERR_INTERNAL);
			
			active_job = 0;
			if (j->error())
				((KIO::Job*)j)->ui()->showErrorMessage();
	
			// shit happened cancel all previous moves
			err = true;
			recover(j->error() != KIO::ERR_FILE_ALREADY_EXIST && j->error() != KIO::ERR_IDENTICAL_FILES);
		}
		else
		{
			success.insert(active_src,active_dst);
			active_src = active_dst = QString();
			active_job = 0;
			startMoving();
		}
	}
	
	void MoveDataFilesJob::onCanceled(KJob* j)
	{
		Q_UNUSED(j);
		setError(KIO::ERR_USER_CANCELED);
		active_job = 0;
		err = true;
		recover(true);
	}
	
	void MoveDataFilesJob::start()
	{
		startMoving();
	}
	
	
	void MoveDataFilesJob::kill(bool quietly)
	{
		Q_UNUSED(quietly);
		// don't do anything we cannot abort in the middle of this operation
	}

	
	void MoveDataFilesJob::startMoving()
	{
		if (todo.isEmpty())
		{
			emitResult();
			return;
		}
			
		QMap<QString,QString>::iterator i = todo.begin();	
		active_job = KIO::file_move(KUrl(i.key()),KUrl(i.value()),-1,KIO::HideProgressInfo);
		active_src = i.key();
		active_dst = i.value();
		Out(SYS_GEN|LOG_DEBUG) << "Moving " << active_src << " -> " << active_dst << endl;
		connect(active_job,SIGNAL(result(KJob*)),this,SLOT(onJobDone(KJob*)));
		connect(active_job,SIGNAL(canceled(KJob*)),this,SLOT(onCanceled(KJob*)));
		todo.erase(i);
	}
	
	void MoveDataFilesJob::recover(bool delete_active)
	{
		if (delete_active && bt::Exists(active_dst))
			bt::Delete(active_dst,true);
		
		if (success.isEmpty())
		{
			emitResult();
			return;
		}
		
		running_recovery_jobs = 0;
		QMap<QString,QString>::iterator i = success.begin();
		while (i != success.end())
		{	
			KIO::Job* j = KIO::file_move(KUrl(i.value()),KUrl(i.key()),-1,KIO::HideProgressInfo);
			connect(j,SIGNAL(result(KJob*)),this,SLOT(onRecoveryJobDone(KJob*)));
			running_recovery_jobs++;
			i++;
		}
		success.clear();
	}
	
	void MoveDataFilesJob::onRecoveryJobDone(KJob* j)
	{
		Q_UNUSED(j);
		running_recovery_jobs--;
		if (running_recovery_jobs <= 0)
			emitResult();
	}
}
#include "movedatafilesjob.moc"
